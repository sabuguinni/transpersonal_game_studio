#include "Quest_DialogueManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "../TranspersonalCharacter.h"

UQuest_DialogueManager::UQuest_DialogueManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CurrentDialogueID = -1;
    bDialogueActive = false;
    DialogueRange = 300.0f;
    CurrentNPC = nullptr;

    // Initialize basic dialogue entries for prehistoric NPCs
    FQuest_DialogueEntry WelcomeDialogue;
    WelcomeDialogue.DialogueID = 1;
    WelcomeDialogue.NPCName = TEXT("Tribal Elder");
    WelcomeDialogue.DialogueText = TEXT("Welcome, young hunter. The great beasts roam these lands. Are you prepared to face them?");
    WelcomeDialogue.bIsQuestDialogue = true;
    WelcomeDialogue.AssociatedQuestID = TEXT("HUNT_FIRST_DINOSAUR");
    
    FQuest_DialogueOption Option1;
    Option1.OptionText = TEXT("I am ready to hunt the great beasts!");
    Option1.NextDialogueID = 2;
    Option1.RewardExperience = 10;
    
    FQuest_DialogueOption Option2;
    Option2.OptionText = TEXT("I need more preparation first.");
    Option2.NextDialogueID = 3;
    
    WelcomeDialogue.PlayerOptions.Add(Option1);
    WelcomeDialogue.PlayerOptions.Add(Option2);
    DialogueDatabase.Add(WelcomeDialogue);

    FQuest_DialogueEntry HuntDialogue;
    HuntDialogue.DialogueID = 2;
    HuntDialogue.NPCName = TEXT("Tribal Elder");
    HuntDialogue.DialogueText = TEXT("Good! Take this stone spear. Hunt a Raptor and bring back its claw as proof of your skill.");
    HuntDialogue.bIsQuestDialogue = true;
    HuntDialogue.AssociatedQuestID = TEXT("HUNT_RAPTOR_CLAW");
    
    FQuest_DialogueOption HuntOption;
    HuntOption.OptionText = TEXT("I will return with the claw.");
    HuntOption.NextDialogueID = -1;
    HuntOption.RewardExperience = 25;
    
    HuntDialogue.PlayerOptions.Add(HuntOption);
    DialogueDatabase.Add(HuntDialogue);

    FQuest_DialogueEntry PrepDialogue;
    PrepDialogue.DialogueID = 3;
    PrepDialogue.NPCName = TEXT("Tribal Elder");
    PrepDialogue.DialogueText = TEXT("Wise choice. Gather 5 stones and 3 sticks first. Then return to me.");
    PrepDialogue.bIsQuestDialogue = true;
    PrepDialogue.AssociatedQuestID = TEXT("GATHER_BASIC_MATERIALS");
    
    FQuest_DialogueOption PrepOption;
    PrepOption.OptionText = TEXT("I will gather the materials.");
    PrepOption.NextDialogueID = -1;
    PrepOption.RewardExperience = 15;
    
    PrepDialogue.PlayerOptions.Add(PrepOption);
    DialogueDatabase.Add(PrepDialogue);
}

void UQuest_DialogueManager::BeginPlay()
{
    Super::BeginPlay();
}

bool UQuest_DialogueManager::StartDialogue(AActor* NPC, int32 StartingDialogueID)
{
    if (!NPC || bDialogueActive)
    {
        return false;
    }

    // Check if player is in range
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), NPC->GetActorLocation());
    if (Distance > DialogueRange)
    {
        return false;
    }

    FQuest_DialogueEntry* DialogueEntry = FindDialogueByID(StartingDialogueID);
    if (!DialogueEntry)
    {
        return false;
    }

    CurrentNPC = NPC;
    CurrentDialogueID = StartingDialogueID;
    bDialogueActive = true;

    return true;
}

void UQuest_DialogueManager::EndDialogue()
{
    bDialogueActive = false;
    CurrentDialogueID = -1;
    CurrentNPC = nullptr;
}

bool UQuest_DialogueManager::SelectDialogueOption(int32 OptionIndex)
{
    if (!bDialogueActive)
    {
        return false;
    }

    FQuest_DialogueEntry* CurrentEntry = FindDialogueByID(CurrentDialogueID);
    if (!CurrentEntry || OptionIndex < 0 || OptionIndex >= CurrentEntry->PlayerOptions.Num())
    {
        return false;
    }

    const FQuest_DialogueOption& SelectedOption = CurrentEntry->PlayerOptions[OptionIndex];

    // Check requirements
    if (!CheckDialogueRequirements(SelectedOption))
    {
        return false;
    }

    // Process rewards
    ProcessDialogueRewards(SelectedOption);

    // Move to next dialogue or end
    if (SelectedOption.NextDialogueID == -1)
    {
        EndDialogue();
    }
    else
    {
        CurrentDialogueID = SelectedOption.NextDialogueID;
    }

    return true;
}

FQuest_DialogueEntry UQuest_DialogueManager::GetCurrentDialogue() const
{
    if (!bDialogueActive)
    {
        return FQuest_DialogueEntry();
    }

    for (const FQuest_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueID == CurrentDialogueID)
        {
            return Entry;
        }
    }

    return FQuest_DialogueEntry();
}

TArray<FQuest_DialogueOption> UQuest_DialogueManager::GetCurrentOptions() const
{
    FQuest_DialogueEntry CurrentEntry = GetCurrentDialogue();
    return CurrentEntry.PlayerOptions;
}

void UQuest_DialogueManager::AddDialogueEntry(const FQuest_DialogueEntry& NewEntry)
{
    DialogueDatabase.Add(NewEntry);
}

void UQuest_DialogueManager::LoadDialoguesFromDataTable(UDataTable* DialogueTable)
{
    if (!DialogueTable)
    {
        return;
    }

    DialogueDatabase.Empty();
    
    TArray<FQuest_DialogueEntry*> AllRows;
    DialogueTable->GetAllRows<FQuest_DialogueEntry>(TEXT("LoadDialogues"), AllRows);
    
    for (FQuest_DialogueEntry* Row : AllRows)
    {
        if (Row)
        {
            DialogueDatabase.Add(*Row);
        }
    }
}

bool UQuest_DialogueManager::CanPlayerAffordOption(const FQuest_DialogueOption& Option) const
{
    return CheckDialogueRequirements(Option);
}

FQuest_DialogueEntry* UQuest_DialogueManager::FindDialogueByID(int32 DialogueID)
{
    for (FQuest_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

void UQuest_DialogueManager::ProcessDialogueRewards(const FQuest_DialogueOption& SelectedOption)
{
    if (SelectedOption.RewardExperience > 0)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(PlayerPawn);
            if (Player)
            {
                // Add experience to player (assuming TranspersonalCharacter has experience system)
                // Player->AddExperience(SelectedOption.RewardExperience);
            }
        }
    }
}

bool UQuest_DialogueManager::CheckDialogueRequirements(const FQuest_DialogueOption& Option) const
{
    if (!Option.bRequiresItem)
    {
        return true;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }

    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(PlayerPawn);
    if (!Player)
    {
        return false;
    }

    // Check if player has required item in inventory
    // This would need to be implemented based on the inventory system
    // For now, return true as placeholder
    return true;
}