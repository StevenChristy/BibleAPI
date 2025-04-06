#include "BibleAPIBPLibrary.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

UBibleDatabase *UBibleAPIBPLibrary::OpenBibleDatabase(const FString& Translation)
{
    FString PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("BibleAPI"))->GetBaseDir();
    FString DBPath = FPaths::Combine(PluginBaseDir, TEXT("Bibles/") + Translation + TEXT(".db"));
    
    if (!FPaths::FileExists(DBPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Bible database not found at: %s"), *DBPath);
        return nullptr;
    }
    
    UBibleDatabase* DB = NewObject<UBibleDatabase>();
    if (DB && DB->Open(DBPath, Translation))
        return DB;
    
    UE_LOG(LogTemp, Error, TEXT("Failed to open Bible database at: %s"), *DBPath);
    return nullptr;
}
