// Fill out your copyright notice in the Description page of Project Settings.
#include "BullCowCartridge.h"
#include "HiddenWordList.h"

void UBullCowCartridge::BeginPlay() // When the game starts
{
    Super::BeginPlay();
    Isograms = GetValidWords(Words);
    SetupGame(); //Setup game

    //PrintLine(FString::Printf(TEXT("Formatting strings and ints to be able to read data through identifiers like HiddenWord as %s and Hiddenword.Len() as %i"), *HiddenWord, HiddenWord.Len()));         ////////////// Pro tip
    

}

void UBullCowCartridge::OnInput(const FString& Input) // When the player hits enter
{
    // Notes on fundamental stuff and weird stuff related to UE4 and C++
    /*int32 a = 1;                                                                                                                                                                Pre vs post increment tip
      int32 b = a++;

      PrintLine(TEXT(%i, %i), a, b);   results in a=2 b=1                                                                                                           Post incrementing changes the original value but returns the original value

      int32 b = ++a;          then,
      PrintLine(TEXT(%i, %i), a, b);   results in a=2 b=2                                                                                                           Pre incrementing changes the original value and then returns this new value

      int32 c = ++ ++a;
      PrintLine(TEXT(%i, %i, %i), a, b, c);   results in a=4 b=2 c=4                                                                                                Don't do this in general

      int32 d = a +=2;                        Better alternative to above
      PrintLine(TEXT(%i, %i, %i, %i), a, b, c, d);   results in a=6 b=2 c=4 d=6

      int32 e = a++;
      PrintLine(TEXT(%i, %i, %i, %i, %i), a, b, c, d, e);   results in a=7 b=2 c=4 d=6 e=6                                                                          a is 6 then gets incremented resulting in 7. e is the return of the value before the change, hence 6
    */

   /* TArray functions
    TArray<FString> bruh;                                                                                                      Add and Emplace do the same thing (append item to end of array) but Emplace does not create a temporary variable in the process
    bruh.Add(TEXT("Hello"));
    bruh.Emplace(TEXT("Hello"));                                                                                                Avoid creating temp variables whenever you can, especially with non-trivial value types like FString
                                                                                                                                Emplace is as efficient as Add so either works. May as well use Emplace over Add, but add is more intuitively understood; fine for trivial types
    bruh.Remove(TEXT("Hello"));                                                                                                 Removes all matching elements from array
    bruh.RemoveAt(bruh[3]);                                                                                                     Removes element at that index
   */

    /*  References
        FString boi = "TEXT("Boi");
        SomeFunction(boi);                                  Function passes in a copy of the value of this variable to use in itself. Better to pass by reference; pass the original variable value in, to save memory whenever possible
        ---------------------------
        void SomeFunction(const FString& SomeWord) {}       const ensures this original variable does not get edited anywhere in the function. Placement of & means pass by reference. Must declare in both header and cpp file.
                                                            Maybe you do want this original value to change at some point. Then don't make the variable const. 
    */

    //if game is over, clear screen + setup game
    //else check player guess
    if (bGameOver)
    {
        ClearScreen();
        SetupGame();
    }
    
    else  //Checking guess
    {
        ProcessGuess(Input);
        
    }
}

void UBullCowCartridge::SetupGame() 
{
    HiddenWord = Isograms[FMath::RandRange( 0, Isograms.Num()-1) ];
    //FString is a TArray of TChar
    // {'c','a','k','e','s','\0'}      array length of 6 but FString.Len() excludes the null at the end
    Lives = HiddenWord.Len();
    bGameOver = false;

    PrintLine(TEXT("Hi There! Welcome to Bull Cows!"));
    PrintLine(TEXT("Guess the %i letter word!"), HiddenWord.Len());
    PrintLine(TEXT("You have %i lives."), Lives);
    PrintLine(TEXT("Please enter your guess then press ENTER"));
     PrintLine(TEXT("The hidden word is %s."), *HiddenWord); // debug line
}   

void UBullCowCartridge::EndGame()
{
    bGameOver = true;
    PrintLine(TEXT("\nPress ENTER to play again"));
}

void UBullCowCartridge::ProcessGuess(const FString& Guess) 
{
    if (Guess == HiddenWord) 
    {
        PrintLine(TEXT("You got the correct word!"));
        EndGame();
        return;
    }

    //Check right number of characters
    if (Guess.Len() != HiddenWord.Len())
    {
        PrintLine(TEXT("Wait! The hidden word is %i letters long."), HiddenWord.Len());
        return;
    }

    //Check isogram
    if (!IsIsogram(Guess))
    {
        PrintLine(TEXT("No repeating letters, try again!"));
        return;
    }

    //Lose a life 
    --Lives;
    PrintLine(TEXT("Lost a life!"));
    
    
    if(Lives <= 0)
    {
        ClearScreen();
        PrintLine(TEXT("You have no lives left!"));
        PrintLine(TEXT("The hidden word was: %s"), *HiddenWord);
        EndGame();
        return;
    }
 
    //Show the bulls and cows

    FBullCowCount Score = GetBullCows(Guess);

    PrintLine(TEXT("You have %i Bulls and %i Cows."), Score.Bulls, Score.Cows);

    PrintLine(TEXT("Guess again! You have %i lives remaining."), Lives);

}

bool UBullCowCartridge::IsIsogram(const FString& Word) const
{
    for (int32 Index = 0; Index < Word.Len(); Index++)
    {
        for (int32 Comparison = Index+1; Comparison < Word.Len(); Comparison++)
        {
            if (Word[Index] == Word[Comparison])
            {
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UBullCowCartridge::GetValidWords(const TArray<FString>& WordList) const
{
    TArray<FString> ValidWords;

    // Range based for loop - Usually good for going through an entire collection one by one
    // for (Type VarName: Data) {
    //    do something
    //}

    for (FString Word : WordList)
    {
        if (Word.Len() >= 4 && Word.Len() <= 8 && IsIsogram(Word))
        {
            ValidWords.Emplace(Word);
        }
    }
    return ValidWords;

}

FBullCowCount UBullCowCartridge::GetBullCows(const FString& Guess) const
{
    //Bulls are right letters in the same position as in the hidden word                      word: ape          Bulls would be any 3 letter word that begins with a, have a second letter that is p, and/or third letter being e; ie: apo, upe
    //Cows are right letters that are not in thr right position as thr hidden word              word: boi       Cows would be the lettes b, o, i, in a different order; ie: iob, oib, etc

    FBullCowCount Count;

    for (int32 GuessIndex = 0; GuessIndex < Guess.Len(); GuessIndex++)
    {
        if (Guess[GuessIndex] == HiddenWord[GuessIndex])
        {
            Count.Bulls++;
            continue;                                                                               // continue goes onto a new iteration, skipping whatever else would have happen in the loop for that current iteration
        }
        for (int32 HiddenIndex = 0; HiddenIndex < HiddenWord.Len(); HiddenIndex++)
        {
            if (Guess[GuessIndex] == HiddenWord[HiddenIndex])
            {
                Count.Cows++;
                break;
            }
            
        }
        
        
    }
    return Count;
}