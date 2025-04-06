// BibleAPI 
#include "BibleDatabase.h"

#include "BibleAPIBPLibrary.h"

/* Schema:

CREATE TABLE IF NOT EXISTS "ASV_books" (
    "id"	INTEGER,
    "name"	TEXT,
    PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "ASV_verses" (
    "id"	INTEGER,
    "book_id"	INTEGER,
    "chapter"	INTEGER,
    "verse"	INTEGER,
    "text"	TEXT,
    PRIMARY KEY("id" AUTOINCREMENT),
    FOREIGN KEY("book_id") REFERENCES "ASV_books"("id")
);
CREATE TABLE IF NOT EXISTS "ASV_word_occurrences" (
    "id"	INTEGER,
    "word_id"	INTEGER,
    "verse_id"	INTEGER,
    "position"	INTEGER,
    PRIMARY KEY("id" AUTOINCREMENT),
    FOREIGN KEY("verse_id") REFERENCES "ASV_verses"("id"),
    FOREIGN KEY("word_id") REFERENCES "ASV_words"("id")
);
CREATE TABLE IF NOT EXISTS "ASV_words" (
    "id"	INTEGER,
    "word"	TEXT UNIQUE,
    "is_proper_noun"	BOOLEAN,
    "frequency"	INTEGER DEFAULT 0,
    PRIMARY KEY("id" AUTOINCREMENT)
);
CREATE TABLE IF NOT EXISTS "translations" (
    "translation"	TEXT,
    "title"	TEXT,
    "license"	TEXT,
    PRIMARY KEY("translation")
);
CREATE INDEX IF NOT EXISTS "idx_ASV_word_occurrences_verse_id" ON "ASV_word_occurrences" (
    "verse_id"
);
CREATE INDEX IF NOT EXISTS "idx_ASV_word_occurrences_word_id" ON "ASV_word_occurrences" (
    "word_id"
);
CREATE INDEX IF NOT EXISTS "idx_ASV_words_word" ON "ASV_words" (
    "word"
);
 */

bool UBibleDatabase::Open(FString Path, FString TranslationCode)
{
    bool BibleOpened = BibleDB.Open(*Path, ESQLiteDatabaseOpenMode::ReadOnly);
    if (BibleOpened)
    {
        FString SQL;

        Translation = TranslationCode;
        
        SQL = TEXT("SELECT id, word, is_proper_noun, frequency FROM ") + TranslationCode + TEXT("_words ORDER BY RANDOM() * frequency LIMIT 1");
        GetRandomWordFrequencyWeightedQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT id, word, is_proper_noun, frequency FROM ") + TranslationCode + TEXT("_words ORDER BY RANDOM() LIMIT 1"); 
        GetRandomWordQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT id, book_id, chapter, verse, text FROM ") + TranslationCode + TEXT("_verses WHERE book_id = ? AND chapter = ? ORDER BY RANDOM() LIMIT 1"); 
        GetRandomVerseQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT id, book_id, chapter, verse, text FROM ") + TranslationCode + TEXT("_verses WHERE book_id = ? AND chapter = ? ORDER BY verse"); 
        GetVersesQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT b.id, b.word, b.is_proper_noun, b.frequency FROM ") + TranslationCode + TEXT("_word_occurrences a WHERE verse_id = ? INNER JOIN ") + TranslationCode + TEXT("_words b ON b.id = a.word_id ORDER BY position");
        GetVerseWordsQuery.Create(BibleDB, *SQL);
        
        SQL = TEXT("SELECT id, word, is_proper_noun, frequency FROM ") + TranslationCode + TEXT("_words WHERE word = ?");
        GetWordQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT verse_id FROM ") + TranslationCode + TEXT("_word_occurrences a WHERE word_id = ? ORDER BY verse_id");
        GetVerseIDsFromWordIDQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT id, book_id, chapter, verse, text FROM ") + TranslationCode + TEXT("_verses WHERE id = ?"); 
        GetVerseByIDQuery.Create(BibleDB, *SQL);

        SQL = TEXT("SELECT id, word, is_proper_noun, frequency FROM ") + TranslationCode + TEXT("_words WHERE id = ?");
        GetWordByIDQuery.Create(BibleDB, *SQL);
        
        Books.Reset();
        FSQLitePreparedStatement BooksQuery;
        SQL = TEXT("SELECT a.id, a.name, max(b.chapter) FROM ") + TranslationCode + TEXT("_books a INNER JOIN " + TranslationCode +"_verses b ON b.book_id = a.id GROUP BY a.id, a.name ORDER BY a.id");
        if (BooksQuery.Create(BibleDB, *SQL))
        {
            if (BooksQuery.IsValid() && !BooksQuery.IsActive())
            {
                if (BooksQuery.Execute())
                {
                    while (BooksQuery.Step() == ESQLitePreparedStatementStepResult::Row)
                    {
                        FBibleBook Book;
                        BooksQuery.GetColumnValueByIndex(0, Book.BookID);
                        BooksQuery.GetColumnValueByIndex(1, Book.BookName);
                        BooksQuery.GetColumnValueByIndex(2, Book.Chapters);
                        Books.Add(Book.BookName) = Book;
                        BookNames.Add(Book.BookName);
                    }
                    BooksQuery.Reset();
                }
            }
            BooksQuery.Destroy();
        }
        
    }
    return BibleOpened;
}

bool UBibleDatabase::GetRandomWord(bool bUseFrequencyWeighting, FBibleWord &RandomWord)
{
    if (!BibleDB.IsValid())
        return false;

    FSQLitePreparedStatement &Query = bUseFrequencyWeighting ? GetRandomWordFrequencyWeightedQuery : GetRandomWordQuery;
    bool Result = false;
    if (Query.Execute())
    {
        if (Query.Step() == ESQLitePreparedStatementStepResult::Row)
        {
            Query.GetColumnValueByIndex(0, RandomWord.WordID);
            Query.GetColumnValueByIndex(1, RandomWord.Word);
            Query.GetColumnValueByIndex(2, RandomWord.IsProperNoun);
            Query.GetColumnValueByIndex(3, RandomWord.Frequency);
            Result = true;
        }
        Query.Reset();
    }

    return Result;
}

FString UBibleDatabase::GetRandomBook()
{
    const int32 Index = FMath::RandRange(0, BookNames.Num() - 1);
    return BookNames[Index];
}

int32 UBibleDatabase::GetRandomChapter(FString BookName)
{
    if (FBibleBook *Book = Books.Find(BookName))
        return FMath::RandRange(1, Book->Chapters);
    return 0;
}

bool UBibleDatabase::GetRandomVerse(FString BookName, int32 Chapter, FBibleVerse& RandomVerse)
{
    bool Result = false;
    if (FBibleBook *Book = Books.Find(BookName))
    {
        if (Chapter <= Book->Chapters && Chapter >= 1)
        {
            GetRandomVerseQuery.SetBindingValueByIndex(1,Book->BookID);
            GetRandomVerseQuery.SetBindingValueByIndex(2,Chapter);
            if (GetRandomVerseQuery.Execute())
            {
                if (GetRandomVerseQuery.Step() == ESQLitePreparedStatementStepResult::Row)
                {
                    GetRandomVerseQuery.GetColumnValueByIndex(0, RandomVerse.VerseID);
                    GetRandomVerseQuery.GetColumnValueByIndex(1, RandomVerse.BookID);
                    GetRandomVerseQuery.GetColumnValueByIndex(2, RandomVerse.Chapter);
                    GetRandomVerseQuery.GetColumnValueByIndex(3, RandomVerse.Verse);
                    GetRandomVerseQuery.GetColumnValueByIndex(4, RandomVerse.Text);
                    Result = true;
                }
                GetRandomVerseQuery.Reset();
            }
            GetRandomVerseQuery.ClearBindings();
        }
    }
    return Result;
}

bool UBibleDatabase::GetVerses(FString BookName, int32 Chapter, TArray<FBibleVerse>& Verses)
{
    bool Result = false;
    if (FBibleBook *Book = Books.Find(BookName))
    {
        if (Chapter <= Book->Chapters && Chapter >= 1)
        {
            GetVersesQuery.SetBindingValueByIndex(1,Book->BookID);
            GetVersesQuery.SetBindingValueByIndex(2,Chapter);
            
            if (GetVersesQuery.Execute())
            {
                while (GetVersesQuery.Step() == ESQLitePreparedStatementStepResult::Row)
                {
                    FBibleVerse &Verse = Verses.Emplace_GetRef();
                    GetVersesQuery.GetColumnValueByIndex(0, Verse.VerseID);
                    GetVersesQuery.GetColumnValueByIndex(1, Verse.BookID);
                    GetVersesQuery.GetColumnValueByIndex(2, Verse.Chapter);
                    GetVersesQuery.GetColumnValueByIndex(3, Verse.Verse);
                    GetVersesQuery.GetColumnValueByIndex(4, Verse.Text);
                    Result = true;
                }
                GetVersesQuery.Reset();
            }
            GetVersesQuery.ClearBindings();
        }
    }
    return Result;
}

bool UBibleDatabase::GetVerseWords(int32 VerseID, TArray<FBibleWord>& Words)
{
    if (!BibleDB.IsValid())
        return false;

    bool Result = false;
    if (GetVerseWordsQuery.IsValid())
    {
        GetVersesQuery.SetBindingValueByIndex(1,VerseID);
        if (GetVerseWordsQuery.Execute())
        {
            while (GetVerseWordsQuery.Step() == ESQLitePreparedStatementStepResult::Row)
            {
                FBibleWord &Word = Words.Emplace_GetRef();
                GetVerseWordsQuery.GetColumnValueByIndex(0, Word.WordID);
                GetVerseWordsQuery.GetColumnValueByIndex(1, Word.Word);
                GetVerseWordsQuery.GetColumnValueByIndex(2, Word.IsProperNoun);
                GetVerseWordsQuery.GetColumnValueByIndex(3, Word.Frequency);
                Result = true;
            }
            GetVerseWordsQuery.Reset();
        }
        GetVerseWordsQuery.ClearBindings();
    }

    return Result;
}

TArray<int32> UBibleDatabase::ExtractWordIDs(const TArray<FBibleWord>& Words)
{
    TArray<int32> Result;
    const FBibleWord *WordsPtr = Words.GetData();
    for (int32 i = 0; i < Words.Num(); i++)
    {
        Result.Add(WordsPtr->WordID);
        WordsPtr++;
    }
    return Result;
}

bool UBibleDatabase::GetWord(FString Word, FBibleWord& BibleWord)
{
    if (!BibleDB.IsValid())
        return false;

    bool Result = false;
    if (GetWordQuery.IsValid())
    {
        GetWordQuery.SetBindingValueByIndex(1,Word);
        if (GetWordQuery.Execute())
        {
            if (GetWordQuery.Step() == ESQLitePreparedStatementStepResult::Row)
            {
                GetWordQuery.GetColumnValueByIndex(0, BibleWord.WordID);
                GetWordQuery.GetColumnValueByIndex(1, BibleWord.Word);
                GetWordQuery.GetColumnValueByIndex(2, BibleWord.IsProperNoun);
                GetWordQuery.GetColumnValueByIndex(3, BibleWord.Frequency);
                Result = true;
            }
            GetWordQuery.Reset();
        }
        GetWordQuery.ClearBindings();
    }

    return Result;
}

TArray<int32> UBibleDatabase::GetVerseIDsFromWordID(int32 WordID)
{
    TArray<int32> Result;
    
    if (GetVerseIDsFromWordIDQuery.IsValid())
    {
        GetVerseIDsFromWordIDQuery.SetBindingValueByIndex(1,WordID);
        if (GetVerseIDsFromWordIDQuery.Execute())
        {
            while (GetVerseIDsFromWordIDQuery.Step() == ESQLitePreparedStatementStepResult::Row)
            {
                int32 VerseID = 0;
                if (GetVerseIDsFromWordIDQuery.GetColumnValueByIndex(0, VerseID))
                    Result.Add(VerseID);
            }
            GetVerseIDsFromWordIDQuery.Reset();
        }
        GetVerseIDsFromWordIDQuery.ClearBindings();
    }

    return Result;
}

TArray<int32> UBibleDatabase::GetVerseIDsWithWordIDs(const TArray<int32>& WordIDs)
{
    // There are inefficiencies with this approach, but the code is easy to understand. We basically want verses matching all the words provided. The more words given the less verses that will be matched.
    // Optimization for this is not trivial.
    TArray<int32> Result;
    
    const int32 *WordID = WordIDs.GetData();
    for (int32 i = 0; i < WordIDs.Num(); i++)
    {
        if (i == 0)
        {
            Result = GetVerseIDsFromWordID(*WordID++);
        }
        else
        {
            Result = UBibleAPIBPLibrary::GetIntersectionOfSortedIntegerArrays(Result, GetVerseIDsFromWordID(*WordID++));
        }
    }

    return Result;
}

bool UBibleDatabase::GetVerseByID(int32 VerseID, FBibleVerse& BibleVerse)
{
    bool Result = false;
    if (GetVerseByIDQuery.IsValid())
    {
        GetVerseByIDQuery.SetBindingValueByIndex(1,VerseID);
        
        if (GetVerseByIDQuery.Execute())
        {
            if (GetVerseByIDQuery.Step() == ESQLitePreparedStatementStepResult::Row)
            {
                GetVerseByIDQuery.GetColumnValueByIndex(0, BibleVerse.VerseID);
                GetVerseByIDQuery.GetColumnValueByIndex(1, BibleVerse.BookID);
                GetVerseByIDQuery.GetColumnValueByIndex(2, BibleVerse.Chapter);
                GetVerseByIDQuery.GetColumnValueByIndex(3, BibleVerse.Verse);
                GetVerseByIDQuery.GetColumnValueByIndex(4, BibleVerse.Text);
                Result = true;
            }
            GetVerseByIDQuery.Reset();
        }
        GetVerseByIDQuery.ClearBindings();
    }
    return Result;
}

bool UBibleDatabase::GetWordByID(int32 WordID, FBibleWord& BibleWord)
{
    bool Result = false;
    if (GetWordByIDQuery.IsValid())
    {
        GetWordByIDQuery.SetBindingValueByIndex(1,WordID);
        if (GetWordByIDQuery.Execute())
        {
            if (GetWordByIDQuery.Step() == ESQLitePreparedStatementStepResult::Row)
            {
                GetWordByIDQuery.GetColumnValueByIndex(0, BibleWord.WordID);
                GetWordByIDQuery.GetColumnValueByIndex(1, BibleWord.Word);
                GetWordByIDQuery.GetColumnValueByIndex(2, BibleWord.IsProperNoun);
                GetWordByIDQuery.GetColumnValueByIndex(3, BibleWord.Frequency);
                Result = true;
            }
            GetWordByIDQuery.Reset();
        }
        GetWordByIDQuery.ClearBindings();
    }
    return Result;
}

void UBibleDatabase::BeginDestroy()
{
    if (BibleDB.IsValid())
    {
        GetRandomWordFrequencyWeightedQuery.Destroy();
        GetRandomWordQuery.Destroy();
        GetRandomVerseQuery.Destroy();
        GetVersesQuery.Destroy();
        GetVerseWordsQuery.Destroy();
        GetWordQuery.Destroy();
        GetVerseIDsFromWordIDQuery.Destroy();
        GetVerseByIDQuery.Destroy();
        GetWordByIDQuery.Destroy();
        BibleDB.Close();
    }
    UObject::BeginDestroy();
}
