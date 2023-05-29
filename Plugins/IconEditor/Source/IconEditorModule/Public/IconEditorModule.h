// Copyright Epic Games, Inc. All Rights Reserved.

#pragma  once

#include "Modules/ModuleManager.h"
#define ENABLE_DEBUG_PRINTING 1

/**
 * The public interface to this module
 */
class IIconEditorModule : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IIconEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IIconEditorModule>("IconEditorModule");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		
		return FModuleManager::Get().IsModuleLoaded("IconEditorModule");
	}

	TWeakObjectPtr<UIconEditor> IconEditor;
};


class FIconEditorModule : public IIconEditorModule
{
public:
	//void ResetEditorPtr() { AssetEditor = nullptr; };
	void OpenIconEditor();
	void ResetIconEditor() { IconEditor.Reset(); };

protected:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	TSharedRef<FExtender> MyAssetViewExtender(const TArray<FAssetData>& AssetInfo);


private:
	
};