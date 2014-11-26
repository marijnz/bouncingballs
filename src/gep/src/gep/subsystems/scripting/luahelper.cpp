#include "stdafx.h"
#include "gep/scripting/luaHelper.h"
#include "gep/globalManager.h"
#include "gep/settings.h"

namespace helper
{
    void indent(const size_t level, std::ostringstream& output)
    {
        for (size_t i = 0; i < level; i++)
        {
            output << "  ";
        }
    }
}

void lua::utils::printType(TraversalInfo input)
{
    auto luaType = lua_type(input.L, input.index);
    auto luaTypeName = lua_typename(input.L, luaType);

    if (input.indent)
    {
        helper::indent(input.level, input.output);
    }

    input.output << luaTypeName;
    switch (luaType)
    {
    case LUA_TNIL:
        break;
    case LUA_TBOOLEAN:
        input.output << '(' << std::boolalpha << lua_toboolean(input.L, input.index) << ')';
        break;
    case LUA_TNUMBER:
        input.output << '(' << lua_tonumber(input.L, input.index) << ')';
        break;
    case LUA_TSTRING:
        input.output << "(\"" << lua_tostring(input.L, input.index) << "\")";
        break;
    case LUA_TTABLE:
        if (input.level < input.maxLevel)
        {
            dumpTable(input);
        }
        else
        {
            input.output << "{<max-level-reached>}";
        }
        break;
    case LUA_TFUNCTION:
    case LUA_TLIGHTUSERDATA:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
        input.output << "(?)";
        break;
    default:
        GEP_ASSERT(false, "Unexpected lua type", luaType, luaTypeName);
        break;
    }
}

void lua::utils::dumpTable(TraversalInfo input)
{
    bool tableIsNotEmpty = false;
    input.output << '{';

    const int top = lua_gettop(input.L);

    const int tableIndex = input.index;

    TraversalInfo keyInput(input);
    keyInput.index = top + 1;
    keyInput.level += 1;
    keyInput.indent = true;

    TraversalInfo valueInput(input);
    valueInput.index = top + 2;
    valueInput.level += 1;
    valueInput.indent = false;

    // first 'dummy' key
    lua_pushnil(input.L);

    // table is in the stack at 'tableIndex'
    while (lua_next(input.L, tableIndex) != 0)
    {
        tableIsNotEmpty = true;
        input.output << '\n';

        printType(keyInput);
        input.output << " => ";
        printType(valueInput);

        // removes 'value' but keeps 'key' for next iteration
        lua_pop(input.L, 1);
    }
    if (tableIsNotEmpty)
    {
        input.output << '\n';
        if (input.indent)
        {
            helper::indent(input.level, input.output);
        }
    }
    input.output << '}';
}

std::string lua::utils::dumpStack(lua_State* L)
{
    std::ostringstream output;
    const size_t level = 0;
    const size_t maxLevel = g_globalManager.getSettings()->getLuaSettings().maxStackDumpLevel;
    int originalTop = lua_gettop(L);

    output << "Size = " << originalTop;

    for (int index = originalTop; index > 0; --index)
    {
        output << "\n[" << index << ',' << -1 + index - originalTop  << "] ";
        printType(TraversalInfo(L, index, level, maxLevel, output));
    }
    output << '\n';

    auto currentTop = lua_gettop(L);
    GEP_ASSERT(originalTop == currentTop, "dumpStack popped too many or too few values!", originalTop, currentTop);

    return output.str();
}

GEP_API std::string lua::utils::traceback(lua_State* L)
{
    StackCleaner cleaner(L, 0);

    lua_getglobal(L, "debug");
    // If global 'debug' is a table, everything is file
    if (lua_istable(L, -1))
    {
        lua_getfield(L, -1, "traceback");
        lua_pushnil(L);
        lua_pushinteger(L, 1);
        lua_call(L, 2, 1);

        return lua_tostring(L, -1);
    }
    return "<Global 'debug' is not a table!>";
}

//////////////////////////////////////////////////////////////////////////

void lua::FunctionWrapper::initializeTable(int functionIndex)
{
    utils::StackCleaner cleaner(m_L, 0);

    // create a table that will store the reference count for us
    lua_createtable(m_L, 2, 0);
    auto tableIndex = lua_gettop(m_L);

    // set the ref count within the table to 1
    lua_pushinteger(m_L, s_refCountIndex);
    lua_pushinteger(m_L, 1);
    lua_rawset(m_L, tableIndex);

    // store the function in the table
    lua_pushinteger(m_L, s_functionIndex);
    lua_pushvalue(m_L, functionIndex);
    lua_rawset(m_L, tableIndex);

    // generate a unique reference to our helper table. Pops the ref'd table.
    m_tableReference = luaL_ref(m_L, LUA_REGISTRYINDEX);
}

void lua::FunctionWrapper::addReference()
{
    utils::StackCleaner cleaner(m_L, 0);

    // push the table on the stack
    pushTable();
    auto tableIndex = lua_gettop(m_L);

    lua_pushinteger(m_L, s_refCountIndex);
    lua_rawget(m_L, tableIndex);

    auto refCount = lua_tointeger(m_L, -1);
    ++refCount;

    lua_pop(m_L, 1);

    lua_pushinteger(m_L, s_refCountIndex);
    lua_pushinteger(m_L, refCount);
    lua_rawset(m_L, tableIndex);
}

void lua::FunctionWrapper::removeReference()
{
    utils::StackCleaner cleaner(m_L, 0);

    // push the table on the stack
    pushTable();
    auto tableIndex = lua_gettop(m_L);

    // get the ref count field
    lua_pushinteger(m_L, s_refCountIndex);
    lua_rawget(m_L, tableIndex);

    // extract the ref count value and decrement the value
    auto refCount = lua_tointeger(m_L, -1);
    --refCount;

    // remove the ref count from the stack
    lua_pop(m_L, 1);

    if (refCount == 0)
    {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableReference);
        m_tableReference = s_invalidReference;
    }
    else
    {
        lua_pushinteger(m_L, s_refCountIndex);
        lua_pushinteger(m_L, refCount);
        lua_rawset(m_L, tableIndex);
    }
}

void lua::FunctionWrapper::push()
{
    if(!isValid())
    {
        lua_pushnil(m_L);
        return;
    }

    utils::StackCleaner cleaner(m_L, 1);

    // Push the table on the stack
    pushTable();
    auto tableIndex = lua_gettop(m_L);

    lua_pushinteger(m_L, s_functionIndex);
    lua_rawget(m_L, tableIndex);

    // Swap the table with the function so that the table is at the top of the stack.
    lua_insert(m_L, -2);
}

void lua::FunctionWrapper::pushTable()
{
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableReference);
}

//////////////////////////////////////////////////////////////////////////

void lua::TableWrapper::push()
{
    if(!isValid())
    {
        lua_pushnil(m_L);
    }
    else
    {
        lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableReference);
    }
}

void lua::TableWrapper::addReference()
{
    utils::StackCleaner cleaner(m_L, 0);

    // push the table on the stack
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableReference);
    auto tableIndex = lua_gettop(m_L);

    // get the ref count field
    lua_pushstring(m_L, refCountIndex());
    lua_rawget(m_L, tableIndex);

    // extract the ref count value and decrement the value
    auto refCount = lua_tointeger(m_L, -1);
    ++refCount;

    // remove the ref count from the stack
    lua_pop(m_L, 1);

    // Push and set the key-value pair: "__refCount" = refCount
    lua_pushstring(m_L, refCountIndex());
    lua_pushinteger(m_L, refCount);
    lua_rawset(m_L, tableIndex);
}

void lua::TableWrapper::removeReference()
{
    utils::StackCleaner cleaner(m_L, 0);

    // push the table on the stack
    lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_tableReference);
    auto tableIndex = lua_gettop(m_L);

    // get the ref count field
    lua_pushstring(m_L, refCountIndex());
    lua_rawget(m_L, tableIndex);

    // extract the ref count value and decrement the value
    auto refCount = lua_tointeger(m_L, -1);
    --refCount;

    // remove the ref count from the stack
    lua_pop(m_L, 1);

    if (refCount == 0)
    {
        luaL_unref(m_L, LUA_REGISTRYINDEX, m_tableReference);
        m_tableReference = s_invalidReference;
    }
    else
    {
        lua_pushstring(m_L, refCountIndex());
        lua_pushinteger(m_L, refCount);
        lua_rawset(m_L, tableIndex);
    }
}

void lua::TableWrapper::injectRefCountEntry(int tableIndex)
{
    utils::StackCleaner cleaner(m_L, 0);

    // set the ref count within the table to 1
    lua_pushstring(m_L, refCountIndex());
    lua_pushinteger(m_L, 1);
    lua_rawset(m_L, tableIndex);

    // generate a unique reference to our helper table. Pops the ref'd table.
    m_tableReference = luaL_ref(m_L, LUA_REGISTRYINDEX);
}

int lua::utils::ptrCompare(lua_State* L)
{
    GEP_ASSERT(lua_gettop(L) == 2, "Expected exactly 2 arguments");
    auto lhsIndex = 1;
    auto rhsIndex = 2;

    lua_getfield(L, lhsIndex, "__ptr");
    auto lhsPtr = lua_touserdata(L, -1);

    lua_getfield(L, rhsIndex, "__ptr");
    auto rhsPtr = lua_touserdata(L, -1);

    lua_pop(L, 4);
    lua_pushboolean(L, lhsPtr == rhsPtr);
    return 1;
}

int lua::utils::nullCheck(lua_State* L)
{
    GEP_ASSERT(lua_gettop(L) == 1, "Expected exactly 1 argument");

    lua_getfield(L, -1, "__ptr");
    GEP_ASSERT(!lua_isnil(L, -1), "Given object has no __ptr field!");
    auto ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);
    lua_pushboolean(L, ptr == nullptr);
    return 1;
}
