# Resource Manager

 ## Explanation:

 The resource manager is a key component of any game engine and handles the loading and unloading of any none-code files including textures fonts and sound files. Later iterations will include loading of XML or JSON formats for levels and other game-related information. The loading and get-ing of the files uses a 'Handle' system which prevents pointers being constantly being passed about. Some design patterns like a singleton would be beneficial here.

---

## Class Structure
###

```c++
{
    class Resource_manager
    {
     private:        
        unsigned int numberOfTextures, numberOfFonts;

        SDL_Renderer* gRenderer;

        std::vector<SDL_Texture*> textures;
        std::vector<const char*> texture_paths;

        std::vector<TTF_Font*> fonts;
        std::vector<font_info*> font_paths;

    public:

        Resource_manager();

        Resource_manager(SDL_Renderer* gRenderer);
        
        //Texture Operations

        unsigned int loadTexture(const char * f_path);
        
        SDL_Texture* getTexture(unsigned int texture_ID);

        void deleteTexture(unsigned int texture_ID);

        //Font Operations

        int loadFont(const char * f_path, int font_size);

        TTF_Font* getFont(unsigned int font_ID);

        void deleteFont(unsigned int font_ID);

};
}
```
---

 # Functions
 ## Resource_manager::Resource_manager(SDL_Renderer* ren)

 Constructor for the resource manager. Takes a pointer to a ```SDL_Renderer``` struct which gets used when creating ```SDL_Texture```s from ```SDL_Surface```s.
 Currently, the constructor sets all the default values for keeping track of currently loaded objects.

---

 ### unsigned int Resource_manager::loadTexture(const char * f_path)
 This function is used to load the texture at the given ```f_path``` on the system's Harddisk. The image is loaded as a ```SDL_Surface``` and stored in the ```Resource_Manager```'s ```textures``` vector. An unsigned integer is returned as the 'Handle' for the texture and is used when another component requires a texture. This function also pushes a copy of the ```f_path``` to texture paths so if the texture is free'd somewhere else in the game loop and needs to be used again it can be just at a cost to cpu time.

 **Just as a precaution: only the resource manager should free and load textures to prevent confusion and keep the system running as fast as possible**

--- 

### SDL_Texture* Resource_manager::getTexture(unsigned int texture_ID)
Returns a ```SDL_Texture*``` based on the texture handle passed to it. If the texture at the specified Handle has been deleted or the pointer is NULL then the function will reload it from the Harddisk

---

### void Resource_manager::deleteTexture(unsigned int texture_ID)
Function to Delete textures from the ```textures``` vector. Sets the pointer value to NULL to prevent any floating pointers or any errors caused from attempting to access an invalid ```SDL_Texture*```.

---

### int Resource_manager::loadFont(const char* f_path, int font_size)

Returns the unsigned integer handle for the font to be loaded at the location specified by ```f_path``` with size ```font_size```. ```Resource_manager::loadFont``` loads the pointer to the font into the ```fonts``` vector and also stores the font size and path in a ```font_info``` struct so that both pieces of information can be kept together.

---

### TTF_Font* Resource_manager::getFont(unsigned int font_ID)

---

### void Resource_manager::deleteFont(unsigned int font_ID)