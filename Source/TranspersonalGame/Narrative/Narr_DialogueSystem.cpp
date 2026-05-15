#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"

UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bDialogueActive = false;
    InteractionRange = 300.0f;
    CurrentDialogueID = "";
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with basic survival dialogues
    FNarr_DialogueEntry GreetingEntry;
    GreetingEntry.DialogueID = "greeting_basic";
    GreetingEntry.SpeakerName = FText::FromString("Tribal Elder");
    GreetingEntry.DialogueText = FText::FromString("You look tired, young hunter. The path of survival is never easy in these lands.");
    GreetingEntry.DialogueType = ENarr_DialogueType::Greeting;
    GreetingEntry.DisplayDuration = 4.0f;
    
    FNarr_DialogueChoice ContinueChoice;
    ContinueChoice.ChoiceText = FText::FromString("Tell me about survival here.");
    ContinueChoice.NextDialogueID = "teaching_survival";
    GreetingEntry.Choices.Add(ContinueChoice);
    
    AddDialogueEntry(GreetingEntry);
    
    // Teaching dialogue
    FNarr_DialogueEntry TeachingEntry;
    TeachingEntry.DialogueID = "teaching_survival";
    TeachingEntry.SpeakerName = FText::FromString("Tribal Elder");
    TeachingEntry.DialogueText = FText::FromString("Listen well - water is life, fire is safety, and shelter is hope. The great beasts respect strength, but they fear cleverness more.");
    TeachingEntry.DialogueType = ENarr_DialogueType::Teaching;
    TeachingEntry.DisplayDuration = 6.0f;
    
    FNarr_DialogueChoice EndChoice;
    EndChoice.ChoiceText = FText::FromString("Thank you for the wisdom.");
    EndChoice.NextDialogueID = "";
    TeachingEntry.Choices.Add(EndChoice);
    
    AddDialogueEntry(TeachingEntry);
}

bool UNarr_DialogueComponent::StartDialogue(const FString& DialogueID)
{
    if (bDialogueActive)
    {
        return false;
    }
    
    FNarr_DialogueEntry* FoundEntry = FindDialogueByID(DialogueID);
    if (!FoundEntry)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dialogue ID not found: %s"), *DialogueID);
        return false;
    }
    
    CurrentDialogueID = DialogueID;
    bDialogueActive = true;
    
    UE_LOG(LogTemp, Log, TEXT("Started dialogue: %s"), *DialogueID);
    return true;
}

void UNarr_DialogueComponent::EndDialogue()
{
    bDialogueActive = false;
    CurrentDialogueID = "";
    
    // Mark dialogue as completed in the global manager
    if (UNarr_DialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueManager>())
    {
        DialogueManager->MarkDialogueCompleted(CurrentDialogueID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Ended dialogue"));
}

FNarr_DialogueEntry UNarr_DialogueComponent::GetCurrentDialogue() const
{
    if (CurrentDialogueID.IsEmpty())
    {
        return FNarr_DialogueEntry();
    }
    
    for (const FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.DialogueID == CurrentDialogueID)
        {
            return Entry;
        }
    }
    
    return FNarr_DialogueEntry();
}

bool UNarr_DialogueComponent::SelectChoice(int32 ChoiceIndex)
{
    if (!bDialogueActive || CurrentDialogueID.IsEmpty())
    {
        return false;
    }
    
    FNarr_DialogueEntry CurrentEntry = GetCurrentDialogue();
    if (ChoiceIndex < 0 || ChoiceIndex >= CurrentEntry.Choices.Num())
    {
        return false;
    }
    
    const FNarr_DialogueChoice& SelectedChoice = CurrentEntry.Choices[ChoiceIndex];
    
    // Check if choice meets conditions
    if (!CheckCondition(SelectedChoice.RequiredCondition, SelectedChoice.ConditionValue))
    {
        UE_LOG(LogTemp, Warning, TEXT("Choice condition not met"));
        return false;
    }
    
    // Move to next dialogue or end
    if (SelectedChoice.NextDialogueID.IsEmpty())
    {
        EndDialogue();
    }
    else
    {
        CurrentDialogueID = SelectedChoice.NextDialogueID;
    }
    
    return true;
}

void UNarr_DialogueComponent::AddDialogueEntry(const FNarr_DialogueEntry& NewEntry)
{
    DialogueEntries.Add(NewEntry);
    
    // Also register with global manager
    if (UNarr_DialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueManager>())
    {
        DialogueManager->RegisterDialogue(NewEntry);
    }
}

bool UNarr_DialogueComponent::CanStartDialogue(AActor* Player) const
{
    if (!Player || bDialogueActive)
    {
        return false;
    }
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= InteractionRange;
}

FNarr_DialogueEntry* UNarr_DialogueComponent::FindDialogueByID(const FString& DialogueID)
{
    for (FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.DialogueID == DialogueID)
        {
            return &Entry;
        }
    }
    return nullptr;
}

bool UNarr_DialogueComponent::CheckCondition(const ENarr_DialogueCondition& Condition, const FString& Value) const
{
    switch (Condition)
    {
        case ENarr_DialogueCondition::None:
            return true;
            
        case ENarr_DialogueCondition::FirstMeeting:
        {
            if (UNarr_DialogueManager* DialogueManager = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueManager>())
            {
                return !DialogueManager->IsDialogueCompleted(Value);
            }
            return true;
        }
        
        case ENarr_DialogueCondition::TimeOfDay:
        {
            // Simple time check - could be expanded with actual time system
            return true;
        }
        
        default:
            return true;
    }
}

// Dialogue Manager Implementation
void UNarr_DialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadPrehistoricDialogues();
    UE_LOG(LogTemp, Log, TEXT("Dialogue Manager initialized"));
}

void UNarr_DialogueManager::RegisterDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    GlobalDialogueDatabase.Add(DialogueEntry.DialogueID, DialogueEntry);
    UE_LOG(LogTemp, Log, TEXT("Registered dialogue: %s"), *DialogueEntry.DialogueID);
}

FNarr_DialogueEntry UNarr_DialogueManager::GetDialogue(const FString& DialogueID) const
{
    if (const FNarr_DialogueEntry* FoundEntry = GlobalDialogueDatabase.Find(DialogueID))
    {
        return *FoundEntry;
    }
    return FNarr_DialogueEntry();
}

void UNarr_DialogueManager::MarkDialogueCompleted(const FString& DialogueID)
{
    CompletedDialogues.AddUnique(DialogueID);
    UE_LOG(LogTemp, Log, TEXT("Marked dialogue completed: %s"), *DialogueID);
}

bool UNarr_DialogueManager::IsDialogueCompleted(const FString& DialogueID) const
{
    return CompletedDialogues.Contains(DialogueID);
}

void UNarr_DialogueManager::SetDialogueFlag(const FString& FlagName, int32 Value)
{
    DialogueFlags.Add(FlagName, Value);
}

int32 UNarr_DialogueManager::GetDialogueFlag(const FString& FlagName) const
{
    if (const int32* FoundValue = DialogueFlags.Find(FlagName))
    {
        return *FoundValue;
    }
    return 0;
}

void UNarr_DialogueManager::LoadPrehistoricDialogues()
{
    CreateSurvivalDialogues();
    CreateTeachingDialogues();
    CreateWarningDialogues();
}

void UNarr_DialogueManager::CreateSurvivalDialogues()
{
    // Water finding dialogue
    FNarr_DialogueEntry WaterEntry;
    WaterEntry.DialogueID = "water_teaching";
    WaterEntry.SpeakerName = FText::FromString("Water Guide");
    WaterEntry.DialogueText = FText::FromString("The water runs clear here, but taste it first - the bitter streams carry death. Look for where the plant-eaters drink.");
    WaterEntry.DialogueType = ENarr_DialogueType::Teaching;
    WaterEntry.AudioAssetPath = "/Game/Audio/Dialogue/WaterGuide.mp3";
    RegisterDialogue(WaterEntry);
    
    // Fire keeping dialogue
    FNarr_DialogueEntry FireEntry;
    FireEntry.DialogueID = "fire_teaching";
    FireEntry.SpeakerName = FText::FromString("Flame Keeper");
    FireEntry.DialogueText = FText::FromString("Fire is life. Fire is safety. The flame-keeper's duty is sacred - let the fire die and the tribe dies with it.");
    FireEntry.DialogueType = ENarr_DialogueType::Teaching;
    FireEntry.AudioAssetPath = "/Game/Audio/Dialogue/FlameKeeper.mp3";
    RegisterDialogue(FireEntry);
    
    // Shelter building dialogue
    FNarr_DialogueEntry ShelterEntry;
    ShelterEntry.DialogueID = "shelter_teaching";
    ShelterEntry.SpeakerName = FText::FromString("Builder");
    ShelterEntry.DialogueText = FText::FromString("Build your shelter facing away from the wind, but where you can see the approaches. The great hunters are patient - they will wait for you to sleep.");
    ShelterEntry.DialogueType = ENarr_DialogueType::Teaching;
    RegisterDialogue(ShelterEntry);
}

void UNarr_DialogueManager::CreateTeachingDialogues()
{
    // Hunting wisdom
    FNarr_DialogueEntry HuntEntry;
    HuntEntry.DialogueID = "hunting_wisdom";
    HuntEntry.SpeakerName = FText::FromString("Master Hunter");
    HuntEntry.DialogueText = FText::FromString("The small prey feeds the body, but the great beasts feed the tribe. Learn their patterns - when they drink, where they sleep, how they protect their young.");
    HuntEntry.DialogueType = ENarr_DialogueType::Teaching;
    RegisterDialogue(HuntEntry);
    
    // Tool crafting
    FNarr_DialogueEntry ToolEntry;
    ToolEntry.DialogueID = "tool_crafting";
    ToolEntry.SpeakerName = FText::FromString("Tool Maker");
    ToolEntry.DialogueText = FText::FromString("Sharp stone cuts, but shaped stone kills. Bind it well with sinew and sap. A broken spear in battle means death - check your tools like your life depends on them.");
    ToolEntry.DialogueType = ENarr_DialogueType::Teaching;
    RegisterDialogue(ToolEntry);
}

void UNarr_DialogueManager::CreateWarningDialogues()
{
    // Raptor warning
    FNarr_DialogueEntry RaptorEntry;
    RaptorEntry.DialogueID = "raptor_warning";
    RaptorEntry.SpeakerName = FText::FromString("Scout");
    RaptorEntry.DialogueText = FText::FromString("Pack hunters spotted near the eastern cliffs! Three, maybe four raptors moving in formation. They hunt at dawn and dusk.");
    RaptorEntry.DialogueType = ENarr_DialogueType::Warning;
    RaptorEntry.AudioAssetPath = "/Game/Audio/Dialogue/ScoutWarning.mp3";
    RegisterDialogue(RaptorEntry);
    
    // Weather warning
    FNarr_DialogueEntry WeatherEntry;
    WeatherEntry.DialogueID = "storm_warning";
    WeatherEntry.SpeakerName = FText::FromString("Weather Watcher");
    WeatherEntry.DialogueText = FText::FromString("The sky grows dark and the wind shifts. The great storms bring flooding and drive the predators to higher ground - our ground.");
    WeatherEntry.DialogueType = ENarr_DialogueType::Warning;
    RegisterDialogue(WeatherEntry);
    
    // Territory warning
    FNarr_DialogueEntry TerritoryEntry;
    TerritoryEntry.DialogueID = "territory_warning";
    TerritoryEntry.SpeakerName = FText::FromString("Territory Guard");
    TerritoryEntry.DialogueText = FText::FromString("That is Thunder Lizard territory. The great one marks its domain with deep footprints and broken trees. Enter only if you seek death.");
    TerritoryEntry.DialogueType = ENarr_DialogueType::Warning;
    RegisterDialogue(TerritoryEntry);
}