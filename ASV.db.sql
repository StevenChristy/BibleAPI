BEGIN TRANSACTION;
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
COMMIT;
