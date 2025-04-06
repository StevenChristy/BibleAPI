import os
import sqlite3
import json
import re
import nltk
from nltk.tokenize import word_tokenize
from nltk.tag import pos_tag

# Download required NLTK data with better error handling
def ensure_nltk_resources():
    resources = [('punkt_tab', 'tokenizers'), ('averaged_perceptron_tagger', 'taggers'), ('averaged_perceptron_tagger_eng', 'taggers')]
    for resource, path in resources:
        try:
            nltk.data.find(f'{path}/{resource}')
            print(f"NLTK resource '{resource}' is available.")
        except LookupError:
            print(f"NLTK resource '{resource}' not found. Downloading...")
            nltk.download(resource, quiet=False)
            print(f"Download of '{resource}' complete.")

# Ensure NLTK resources are available before proceeding
ensure_nltk_resources()

def create_word_index(db_path):
    """Create a word index table for the given Bible database."""
    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Get the translation code from the filename
    translation = os.path.basename(db_path).split('.')[0]

    # Check if the word table already exists
    cursor.execute(f"SELECT name FROM sqlite_master WHERE type='table' AND name='{translation}_words'")
    if cursor.fetchone():
        print(f"Word table for {translation} already exists. Dropping and recreating.")
        cursor.execute(f"DROP TABLE IF EXISTS {translation}_words")
        cursor.execute(f"DROP TABLE IF EXISTS {translation}_word_occurrences")
    
    # Create the words table
    cursor.execute(f"""
    CREATE TABLE {translation}_words (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        word TEXT UNIQUE,
        is_proper_noun BOOLEAN,
        frequency INTEGER DEFAULT 0
    );
    """)
    
    # Create the word occurrences table (for tracking where each word appears)
    cursor.execute(f"""
    CREATE TABLE {translation}_word_occurrences (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        word_id INTEGER,
        verse_id INTEGER,
        position INTEGER,
        FOREIGN KEY (word_id) REFERENCES {translation}_words(id),
        FOREIGN KEY (verse_id) REFERENCES {translation}_verses(id)
    );
    """)
    
    # Get all verses
    cursor.execute(f"""
    SELECT v.id, v.text, b.name, v.chapter, v.verse 
    FROM {translation}_verses v
    JOIN {translation}_books b ON v.book_id = b.id
    ORDER BY v.id
    """)
    verses = cursor.fetchall()
    
    # Dictionary to store word IDs
    word_ids = {}
    
    # Process each verse
    for verse_id, text, book, chapter, verse in verses:
        # Tokenize the text
        tokens = word_tokenize(text)
        
        # Get part-of-speech tags
        tagged_tokens = pos_tag(tokens)
        
        # Process each word
        for position, (token, tag) in enumerate(tagged_tokens):
            # Skip punctuation and numbers
            if not re.match(r'^[a-zA-Z]', token):
                continue
            
            # Determine if it's a proper noun
            is_proper_noun = tag.startswith('NNP')
            
            # Normalize the word (keep capitalization for proper nouns)
            if is_proper_noun:
                normalized_word = token
            else:
                normalized_word = token.lower()
            
            # Check if the word is already in the database
            if normalized_word in word_ids:
                word_id = word_ids[normalized_word]
                # Update frequency
                cursor.execute(f"""
                UPDATE {translation}_words 
                SET frequency = frequency + 1 
                WHERE id = ?
                """, (word_id,))
            else:
                # Insert the new word
                cursor.execute(f"""
                INSERT INTO {translation}_words (word, is_proper_noun, frequency) 
                VALUES (?, ?, 1)
                """, (normalized_word, is_proper_noun))
                word_id = cursor.lastrowid
                word_ids[normalized_word] = word_id
            
            # Record the occurrence
            cursor.execute(f"""
            INSERT INTO {translation}_word_occurrences (word_id, verse_id, position)
            VALUES (?, ?, ?)
            """, (word_id, verse_id, position))
        
        # Commit every 1000 verses to avoid large transactions
        if verse_id % 1000 == 0:
            conn.commit()
            print(f"Processed {verse_id} verses... ({book} {chapter}:{verse})")
    
    # Create indexes for better performance
    cursor.execute(f"CREATE INDEX idx_{translation}_words_word ON {translation}_words(word)")
    cursor.execute(f"CREATE INDEX idx_{translation}_word_occurrences_word_id ON {translation}_word_occurrences(word_id)")
    cursor.execute(f"CREATE INDEX idx_{translation}_word_occurrences_verse_id ON {translation}_word_occurrences(verse_id)")
    
    # Commit changes before optimization
    conn.commit()
    
    # Optimization and compaction steps
    print("Optimizing database...")
    
    # Check size before vacuum
    before_size = os.path.getsize(db_path)
    print(f"Database size before VACUUM: {before_size} bytes")
    
    # Analyze the database for better query optimization
    cursor.execute("ANALYZE")
    
    # Vacuum to reclaim space
    cursor.execute("VACUUM")
    
    # Check size after vacuum
    after_size = os.path.getsize(db_path)
    print(f"Database size after VACUUM: {after_size} bytes")
    print(f"Space reclaimed: {before_size - after_size} bytes")

    # Commit changes and close connection
    conn.commit()
    conn.close()
    
    print(f"Word indexing completed for {translation}. Total unique words: {len(word_ids)}")

if __name__ == "__main__":
    # Get db_path from command line argument or use default
    db_path = sys.argv[1] if len(sys.argv) > 1 else "ASV.db"
    if not os.path.exists(db_path):
        print(f"Error: Database file '{db_path}' does not exist.")
        sys.exit(1)
    # Create word index
    create_word_index(db_path)