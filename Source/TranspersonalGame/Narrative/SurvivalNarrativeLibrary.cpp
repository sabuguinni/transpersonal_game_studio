#include "SurvivalNarrativeLibrary.h"
#include "Engine/Engine.h"

TMap<ENarr_SurvivalContext, TArray<FString>> USurvivalNarrativeLibrary::SurvivalDialogueDatabase;
TMap<ENarr_DinosaurType, TArray<FString>> USurvivalNarrativeLibrary::DinosaurDialogueDatabase;
TMap<ENarr_BiomeType, TArray<FString>> USurvivalNarrativeLibrary::EnvironmentDialogueDatabase;

FString USurvivalNarrativeLibrary::GetSurvivalDialogue(ENarr_SurvivalContext Context, float PlayerHealth, float PlayerHunger)
{
    InitializeDialogueDatabases();
    
    if (SurvivalDialogueDatabase.Contains(Context))
    {
        TArray<FString> ContextDialogues = SurvivalDialogueDatabase[Context];
        
        // Modify dialogue based on player stats
        if (PlayerHealth < 0.3f)
        {
            return TEXT("My wounds are severe. I need to find shelter and tend to my injuries before it's too late.");
        }
        else if (PlayerHunger < 0.2f)
        {
            return TEXT("Hunger gnaws at my stomach. I must find food soon or I won't have the strength to continue.");
        }
        
        return SelectRandomDialogue(ContextDialogues);
    }
    
    return TEXT("The wilderness tests my resolve with every step.");
}

FString USurvivalNarrativeLibrary::GetDinosaurEncounterDialogue(ENarr_DinosaurType DinosaurType, float Distance)
{
    InitializeDialogueDatabases();
    
    if (DinosaurDialogueDatabase.Contains(DinosaurType))
    {
        TArray<FString> DinosaurDialogues = DinosaurDialogueDatabase[DinosaurType];
        
        // Modify based on distance
        if (Distance < 500.0f)
        {
            switch (DinosaurType)
            {
                case ENarr_DinosaurType::TRex:
                    return TEXT("The earth trembles under massive footsteps. A apex predator stalks nearby - I must not be seen.");
                case ENarr_DinosaurType::Raptor:
                    return TEXT("Clever eyes watch me from the shadows. These pack hunters are testing my defenses.");
                case ENarr_DinosaurType::Triceratops:
                    return TEXT("A massive herbivore blocks my path. Best to give it wide berth - those horns are not for show.");
                default:
                    return TEXT("A prehistoric giant moves through my territory. Caution is the key to survival.");
            }
        }
        
        return SelectRandomDialogue(DinosaurDialogues);
    }
    
    return TEXT("Ancient beasts roam these lands. I am but a visitor in their domain.");
}

FString USurvivalNarrativeLibrary::GetEnvironmentDialogue(ENarr_BiomeType BiomeType, ENarr_TimeOfDay TimeOfDay)
{
    InitializeDialogueDatabases();
    
    if (EnvironmentDialogueDatabase.Contains(BiomeType))
    {
        TArray<FString> BiomeDialogues = EnvironmentDialogueDatabase[BiomeType];
        
        // Modify based on time of day
        if (TimeOfDay == ENarr_TimeOfDay::Night)
        {
            switch (BiomeType)
            {
                case ENarr_BiomeType::Forest:
                    return TEXT("The forest grows dark and full of unseen dangers. Every shadow could hide a predator.");
                case ENarr_BiomeType::Plains:
                    return TEXT("Under the star-filled sky, the plains stretch endlessly. Nowhere to hide from night hunters.");
                case ENarr_BiomeType::Swamp:
                    return TEXT("The swamp at night is a chorus of unknown sounds. Death lurks beneath every murky pool.");
                default:
                    return TEXT("Night transforms this land into something far more dangerous than it appears by day.");
            }
        }
        
        return SelectRandomDialogue(BiomeDialogues);
    }
    
    return TEXT("This ancient world holds beauty and terror in equal measure.");
}

FString USurvivalNarrativeLibrary::GetCraftingDialogue(ENarr_CraftingAction Action, bool bSuccess)
{
    if (bSuccess)
    {
        switch (Action)
        {
            case ENarr_CraftingAction::MakeTool:
                return TEXT("My hands remember the ancient ways. This tool will serve me well in the trials ahead.");
            case ENarr_CraftingAction::BuildShelter:
                return TEXT("A simple shelter, but it will keep the rain and wind at bay. Progress, one stone at a time.");
            case ENarr_CraftingAction::MakeWeapon:
                return TEXT("Sharp stone and sturdy wood - the difference between predator and prey in this world.");
            default:
                return TEXT("Each creation brings me closer to mastering this harsh environment.");
        }
    }
    else
    {
        return TEXT("The materials resist my efforts. I must learn patience - survival cannot be rushed.");
    }
}

TArray<FNarr_SurvivalDialogue> USurvivalNarrativeLibrary::GetContextualDialogues(ENarr_SurvivalContext Context)
{
    TArray<FNarr_SurvivalDialogue> Dialogues;
    
    // Return pre-built dialogue sets based on context
    if (Context == ENarr_SurvivalContext::Combat)
    {
        FNarr_SurvivalDialogue CombatDialogue;
        CombatDialogue.DialogueID = TEXT("Combat_Preparation");
        CombatDialogue.SpeakerName = TEXT("InnerVoice");
        CombatDialogue.DialogueText = TEXT("My weapon is ready. If I must fight, I will fight to survive.");
        CombatDialogue.Context = Context;
        CombatDialogue.Priority = 2.0f;
        Dialogues.Add(CombatDialogue);
    }
    
    return Dialogues;
}

FString USurvivalNarrativeLibrary::FormatSurvivalStatus(float Health, float Hunger, float Thirst, float Stamina)
{
    FString StatusText = TEXT("Status: ");
    
    if (Health < 0.3f)
        StatusText += TEXT("Wounded, ");
    else if (Health < 0.7f)
        StatusText += TEXT("Injured, ");
    else
        StatusText += TEXT("Healthy, ");
        
    if (Hunger < 0.3f)
        StatusText += TEXT("Starving, ");
    else if (Hunger < 0.7f)
        StatusText += TEXT("Hungry, ");
    else
        StatusText += TEXT("Fed, ");
        
    if (Thirst < 0.3f)
        StatusText += TEXT("Dehydrated, ");
    else if (Thirst < 0.7f)
        StatusText += TEXT("Thirsty, ");
    else
        StatusText += TEXT("Hydrated, ");
        
    if (Stamina < 0.3f)
        StatusText += TEXT("Exhausted");
    else if (Stamina < 0.7f)
        StatusText += TEXT("Tired");
    else
        StatusText += TEXT("Energetic");
    
    return StatusText;
}

void USurvivalNarrativeLibrary::InitializeDialogueDatabases()
{
    static bool bInitialized = false;
    if (bInitialized) return;
    
    // Initialize survival context dialogues
    TArray<FString> GeneralDialogues = {
        TEXT("Each step forward is a victory against this unforgiving world."),
        TEXT("The ancient earth beneath my feet holds countless secrets and dangers."),
        TEXT("I am alone, but not helpless. Survival is a skill that can be learned.")
    };
    SurvivalDialogueDatabase.Add(ENarr_SurvivalContext::General, GeneralDialogues);
    
    TArray<FString> CombatDialogues = {
        TEXT("Violence is sometimes the only language this world understands."),
        TEXT("My primitive weapons must be enough - there is no other choice."),
        TEXT("Fight or flee - the eternal choice of the hunted.")
    };
    SurvivalDialogueDatabase.Add(ENarr_SurvivalContext::Combat, CombatDialogues);
    
    // Initialize dinosaur dialogues
    TArray<FString> TRexDialogues = {
        TEXT("The king of predators stalks these lands. I am nothing more than prey to such a beast."),
        TEXT("Massive jaws that could crush stone. I must avoid this apex predator at all costs.")
    };
    DinosaurDialogueDatabase.Add(ENarr_DinosaurType::TRex, TRexDialogues);
    
    TArray<FString> RaptorDialogues = {
        TEXT("Intelligent eyes study my every move. These pack hunters are calculating my weaknesses."),
        TEXT("Swift and cunning - the perfect predators for hunting prey like me.")
    };
    DinosaurDialogueDatabase.Add(ENarr_DinosaurType::Raptor, RaptorDialogues);
    
    // Initialize environment dialogues
    TArray<FString> ForestDialogues = {
        TEXT("The ancient forest whispers with the sounds of a million creatures."),
        TEXT("Thick canopy blocks the sun - perfect cover for both hunter and hunted.")
    };
    EnvironmentDialogueDatabase.Add(ENarr_BiomeType::Forest, ForestDialogues);
    
    bInitialized = true;
}

FString USurvivalNarrativeLibrary::SelectRandomDialogue(const TArray<FString>& DialogueOptions)
{
    if (DialogueOptions.Num() == 0)
    {
        return TEXT("...");
    }
    
    int32 RandomIndex = FMath::RandRange(0, DialogueOptions.Num() - 1);
    return DialogueOptions[RandomIndex];
}