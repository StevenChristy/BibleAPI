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

TArray<int32> UBibleAPIBPLibrary::GetIntersectionOfSortedIntegerArrays(const TArray<int32>& a, const TArray<int32>& b)
{
    TArray<int32> Result;

    int MaxSetSize = FMath::Min(a.Num(), b.Num());

    if (MaxSetSize == 0)
        return Result;
    
    Result.Reserve(MaxSetSize);

    const int32 *ArrayA = a.GetData();
    const int32 *ArrayB = b.GetData();
    const int32 *MaxA = ArrayA + a.Num();
    const int32 *MaxB = ArrayB + b.Num();

    while (ArrayA < MaxA && ArrayB < MaxB)
    {
        int32 avalue = *ArrayA;
        int32 bvalue = *ArrayB;
        if (avalue == bvalue)
        {
            Result.Add(avalue);
            ArrayA++;
            ArrayB++;
        }
        else if (avalue < bvalue)
            ArrayA++;
        else
            ArrayB++;
    }

    return Result;
}