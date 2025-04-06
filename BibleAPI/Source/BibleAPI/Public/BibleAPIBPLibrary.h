// BibleAPI 
#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BibleDatabase.h"
#include "BibleAPIBPLibrary.generated.h"


UCLASS()
class BIBLEAPI_API UBibleAPIBPLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Initialize the database connection
    UFUNCTION(BlueprintCallable, Category = "BibleAPI")
    static UBibleDatabase *OpenBibleDatabase(const FString& Translation = FString("ASV"));
};