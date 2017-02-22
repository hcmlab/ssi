
#ifndef SSI_ANDROID_APP_H
#define SSI_ANDROID_APP_H

 
//contains Android specific data needed for running ssi in an app

class ssiAndroidApp
{
public:
	ssiAndroidApp()
	{
		libsDir=0;
		app=0;
	}
	~ssiAndroidApp()
	{
		app=0;
		delete[] libsDir;
		libsDir=0;
	}
	
	void setLibDir(char* libsDir)
	{
		delete[] this->libsDir;
		this->libsDir= new char[strlen(libsDir)+1];
		memset(this->libsDir, 0,strlen(libsDir)+1 );
		strcpy(this->libsDir, libsDir);
	}
	char* getLibDir()
	{
		return libsDir;
	}
	void setApp(void* app)
	{
		
		this->app=app;
	}
	void* getApp()
	{
		return app;
	}
	
private:
	
	//ld_library_path
	char* libsDir;
	
	//native activitys application state
	void* app;
	
};

#endif
