#include "Narr_QuestNarrativeIntegration.h"
#include "Narr_DialogueSystem.h"
#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UNarr_QuestNarrativeIntegration::UNarr_QuestNarrativeIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    ContextUpdateInterval = 5.0f;
    EmergencyDialogueCooldown = 30.0f;
    LastContextUpdate = 0.0f;
    LastEmergencyDialogue = 0.0f;
    bIsDialogueActive = false;
    
    DialogueSystem = nullptr;
    StoryManager = nullptr;
}

void UNarr_QuestNarrativeIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize dialogue database with survival-focused content
    InitializeDialogueDatabase();
    
    // Find or create dialogue system
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            DialogueSystem = GameInstance->GetSubsystem<UNarr_DialogueSystem>();
            StoryManager = GameInstance->GetSubsystem<UNarr_StoryManager>();
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest-Narrative Integration initialized"));
}

void UNarr_QuestNarrativeIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateContextualNarration(DeltaTime);
}

void UNarr_QuestNarrativeIntegration::RegisterQuestDialogue(const FString& QuestID, const FString& DialogueText, const FString& CharacterName)
{
    FNarr_QuestDialogueData DialogueData;
    DialogueData.QuestID = QuestID;
    DialogueData.DialogueText = DialogueText;
    DialogueData.CharacterName = CharacterName;
    DialogueData.DialoguePriority = 1.0f;
    DialogueData.bIsEmergencyDialogue = false;
    
    QuestDialogueMap.Add(QuestID, DialogueData);
    
    UE_LOG(LogTemp, Log, TEXT("Registered quest dialogue for: %s"), *QuestID);
}

void UNarr_QuestNarrativeIntegration::TriggerQuestNarrative(const FString& QuestID)
{
    if (QuestDialogueMap.Contains(QuestID))
    {
        const FNarr_QuestDialogueData& DialogueData = QuestDialogueMap[QuestID];
        
        if (DialogueSystem)
        {
            // Trigger contextual dialogue based on current survival state
            FString ContextualDialogue = SelectAppropriateDialogue(QuestID, CurrentSurvivalContext);
            
            UE_LOG(LogTemp, Log, TEXT("Triggering quest narrative: %s - %s"), *QuestID, *ContextualDialogue);
            
            // Play dialogue through the dialogue system
            bIsDialogueActive = true;
        }
    }
}

void UNarr_QuestNarrativeIntegration::UpdateSurvivalContext(const FNarr_SurvivalContextData& ContextData)
{
    CurrentSurvivalContext = ContextData;
    LastContextUpdate = GetWorld()->GetTimeSeconds();
    
    // Check if emergency dialogue should be triggered
    if (ShouldTriggerEmergencyDialogue(ContextData.ThreatLevel))
    {
        PlayEmergencyDialogue(TEXT("HighThreat"), ContextData.ThreatLevel);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated survival context - Health: %.1f, Threat: %.1f"), 
           ContextData.PlayerHealth, ContextData.ThreatLevel);
}

bool UNarr_QuestNarrativeIntegration::PlayQuestDialogue(const FString& QuestID)
{
    if (!QuestDialogueMap.Contains(QuestID) || bIsDialogueActive)
    {
        return false;
    }
    
    const FNarr_QuestDialogueData& DialogueData = QuestDialogueMap[QuestID];
    
    // Generate contextual dialogue based on survival state
    FString FinalDialogue = SelectAppropriateDialogue(QuestID, CurrentSurvivalContext);
    
    if (DialogueSystem)
    {
        bIsDialogueActive = true;
        UE_LOG(LogTemp, Log, TEXT("Playing quest dialogue: %s"), *FinalDialogue);
        
        // Set dialogue inactive after a delay (simulated)
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { bIsDialogueActive = false; },
            5.0f,
            false
        );
        
        return true;
    }
    
    return false;
}

void UNarr_QuestNarrativeIntegration::PlayEmergencyDialogue(const FString& ThreatType, float ThreatLevel)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastEmergencyDialogue < EmergencyDialogueCooldown)
    {
        return; // Still in cooldown
    }
    
    LastEmergencyDialogue = CurrentTime;
    
    FString EmergencyText;
    if (ThreatType == TEXT("HighThreat"))
    {
        if (ThreatLevel > 8.0f)
        {
            EmergencyText = TEXT("CRITICAL THREAT DETECTED! Massive predator approaching. Seek immediate shelter!");
        }
        else if (ThreatLevel > 5.0f)
        {
            EmergencyText = TEXT("Warning: Dangerous predator in the area. Stay alert and avoid open spaces.");
        }
        else
        {
            EmergencyText = TEXT("Caution: Potential threat detected. Recommend increased vigilance.");
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Emergency Dialogue: %s"), *EmergencyText);
    
    if (DialogueSystem)
    {
        bIsDialogueActive = true;
    }
}

void UNarr_QuestNarrativeIntegration::TriggerContextualNarration()
{
    FString NarrationText = GenerateSurvivalTip(CurrentSurvivalContext);
    
    UE_LOG(LogTemp, Log, TEXT("Contextual Narration: %s"), *NarrationText);
    
    if (DialogueSystem && !bIsDialogueActive)
    {
        bIsDialogueActive = true;
        
        // Reset after narration
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            [this]() { bIsDialogueActive = false; },
            3.0f,
            false
        );
    }
}

FString UNarr_QuestNarrativeIntegration::GenerateSurvivalTip(const FNarr_SurvivalContextData& Context)
{
    // Generate contextual survival tips based on current state
    if (Context.PlayerHealth < 30.0f)
    {
        return TEXT("Your health is critically low. Find shelter and rest to recover.");
    }
    
    if (Context.PlayerHunger < 20.0f)
    {
        return TEXT("Hunger is becoming dangerous. Search for edible plants or hunt small prey.");
    }
    
    if (Context.PlayerThirst < 20.0f)
    {
        return TEXT("Dehydration is setting in. Locate a water source immediately.");
    }
    
    if (Context.ThreatLevel > 7.0f)
    {
        return TEXT("Extreme danger detected. Find elevated shelter and avoid movement.");
    }
    
    if (Context.CurrentBiome == TEXT("Forest") && Context.ThreatLevel > 3.0f)
    {
        return TEXT("Forest predators are active. Stick to clearings and avoid dense vegetation.");
    }
    
    return TEXT("Stay alert, survivor. The Cretaceous wilderness shows no mercy to the unprepared.");
}

void UNarr_QuestNarrativeIntegration::InitializeDialogueDatabase()
{
    // Initialize with survival-focused quest dialogues
    RegisterQuestDialogue(
        TEXT("QUEST_FirstShelter"),
        TEXT("Building your first shelter is crucial for survival. Find a defensible position away from predator paths."),
        TEXT("Survival Instructor")
    );
    
    RegisterQuestDialogue(
        TEXT("QUEST_WaterSource"),
        TEXT("Clean water is life in the Cretaceous. Look for flowing streams, but beware of predators that hunt near water."),
        TEXT("Veteran Survivor")
    );
    
    RegisterQuestDialogue(
        TEXT("QUEST_FirstHunt"),
        TEXT("Small prey like Compsognathus can provide food, but even tiny predators can be dangerous in groups."),
        TEXT("Hunter Guide")
    );
    
    RegisterQuestDialogue(
        TEXT("QUEST_AvoidPredators"),
        TEXT("Learn to read the signs - broken branches, claw marks, and droppings tell you what hunts in this area."),
        TEXT("Tracker Mentor")
    );
    
    RegisterQuestDialogue(
        TEXT("QUEST_CraftTools"),
        TEXT("Stone tools are your lifeline. Sharp rocks can mean the difference between eating and becoming food."),
        TEXT("Craftsman")
    );
    
    UE_LOG(LogTemp, Log, TEXT("Dialogue database initialized with %d entries"), QuestDialogueMap.Num());
}

void UNarr_QuestNarrativeIntegration::UpdateContextualNarration(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastContextUpdate > ContextUpdateInterval)
    {
        // Periodically trigger contextual narration based on survival state
        if (!bIsDialogueActive)
        {
            // Check for critical survival conditions
            if (CurrentSurvivalContext.PlayerHealth < 50.0f || 
                CurrentSurvivalContext.PlayerHunger < 30.0f ||
                CurrentSurvivalContext.PlayerThirst < 30.0f)
            {
                TriggerContextualNarration();
            }
        }
        
        LastContextUpdate = CurrentTime;
    }
}

bool UNarr_QuestNarrativeIntegration::ShouldTriggerEmergencyDialogue(float ThreatLevel)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    return (ThreatLevel > 5.0f && 
            CurrentTime - LastEmergencyDialogue > EmergencyDialogueCooldown &&
            !bIsDialogueActive);
}

FString UNarr_QuestNarrativeIntegration::SelectAppropriateDialogue(const FString& QuestID, const FNarr_SurvivalContextData& Context)
{
    if (!QuestDialogueMap.Contains(QuestID))
    {
        return TEXT("Quest dialogue not found.");
    }
    
    FString BaseDialogue = QuestDialogueMap[QuestID].DialogueText;
    
    // Modify dialogue based on context
    if (Context.ThreatLevel > 5.0f)
    {
        BaseDialogue += TEXT(" But be extra cautious - dangerous predators are nearby.");
    }
    
    if (Context.PlayerHealth < 50.0f)
    {
        BaseDialogue += TEXT(" Take care of your wounds first - survival depends on staying healthy.");
    }
    
    return BaseDialogue;
}