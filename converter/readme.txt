Model convert for converting various formats to thModel

A lot of input formats are supported for more details see http://assimp.sourceforge.net/

The recommended input format is Collada (dae)

Command line options
====================

--workdir <path> The root path to be used for relative filenames within the input model. 
  Used to test if texture files exists not neccessary when --includeMissingTextures is specified.

--includeMissingTextures includes all textures specified in the model file even when they are 
  missing on the disc.
  
  
Example usage
==============

ModelConverterD --includeMissingTextures ball.dae

This will output a ball.thModel in the same directory as the ball.dae


Desktop Shortcut
================
Its also possible to create a shortcut to ModelConverterD with the parameters 
attached to it (e.g. --includeMissingTextures) and then drag & drop the input model file onto
that shortcut.


Source
======
If changes to the model converter need to be made the source needs to be converted to c++ first. 
This should not be to much effort as the contents of modeltypes.d and the ChunkFile class 
are already contained within the GEP project.