// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "CoreMinimal.h"
#include "SQLiteDatabase.h"
#include "UObject/Object.h"
#include "BibleDatabase.generated.h"

USTRUCT(BlueprintType)
struct FBibleBook
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 BookID = 0;

	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	FString BookName;

	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 Chapters = 0;
};


USTRUCT(BlueprintType)
struct FBibleVerse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 VerseID = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 BookID = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 Chapter = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 Verse = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	FString Text;
};

USTRUCT(BlueprintType)
struct FBibleWord
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 WordID = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	FString Word;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 IsProperNoun = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="BibleAPI")
	int32 Frequency = 0;
};
/**
 * 
 */
UCLASS()
class BIBLEAPI_API UBibleDatabase : public UObject
{
	GENERATED_BODY()
	
	FSQLiteDatabase BibleDB;
	FSQLitePreparedStatement GetRandomWordQuery;
	FSQLitePreparedStatement GetRandomWordFrequencyWeightedQuery;
	FSQLitePreparedStatement GetRandomVerseQuery;
	FSQLitePreparedStatement GetVersesQuery;
	FSQLitePreparedStatement GetVerseWordsQuery;
	FSQLitePreparedStatement GetWordQuery;

public:
	UPROPERTY(BlueprintReadOnly, Category = "BibleAPI")
	FString Translation;

	UPROPERTY(BlueprintReadOnly, Category = "BibleAPI")
	TMap<FString, FBibleBook> Books;

	UPROPERTY(BlueprintReadOnly, Category = "BibleAPI")
	TArray<FString> BookNames;

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	bool GetRandomWord(bool bUseFrequencyWeighting, FBibleWord &RandomWord);

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	FString GetRandomBook();

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	int32 GetRandomChapter(FString BookName);

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	bool GetRandomVerse(FString BookName, int32 Chapter, FBibleVerse &RandomVerse);

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	bool GetVerses(FString BookName, int32 Chapter, TArray<FBibleVerse> &Verses);

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	bool GetVerseWords(int32 VerseID, TArray<FBibleWord> &Words);

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	TArray<int32> ExtractWordIDs(const TArray<FBibleWord> &Words);

	UFUNCTION(BlueprintCallable, Category = "BibleAPI")
	bool GetWord(FString Word, FBibleWord &BibleWord);
	
	virtual void BeginDestroy() override;
	bool Open(FString Path, FString TranslationCode);
};
