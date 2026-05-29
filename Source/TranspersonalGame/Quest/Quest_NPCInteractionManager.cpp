#include "Quest_NPCInteractionManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "../Crowd/CrowdSimulationManager.h"
#include "../Crowd/Crowd_PrehistoricNPCManager.h"

UQuest_NPCInteractionManager::UQuest_NPCInteractionManager()
{
    CrowdManager = nullptr;
    NPCManager = nullptr;
}

void UQuest_NPCInteractionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Initializing NPC interaction system"));
    
    // Initialize default data
    InitializeDefaultInteractions();
    CreateDefaultTribalQuests();
    SetupDefaultDialogues();
    
    // Find crowd system reference
    if (UWorld* World = GetWorld())
    {
        CrowdManager = Cast<ACrowdSimulationManager>(UGameplayStatics::GetActorOfClass(World, ACrowdSimulationManager::StaticClass()));
        if (CrowdManager)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Found CrowdSimulationManager"));
        }
    }
}

void UQuest_NPCInteractionManager::Deinitialize()
{
    RegisteredNPCs.Empty();
    TribalQuests.Empty();
    QuestTimers.Empty();
    InteractionCounts.Empty();
    
    CrowdManager = nullptr;
    NPCManager = nullptr;
    
    Super::Deinitialize();
}

void UQuest_NPCInteractionManager::RegisterNPCForInteraction(const FString& NPCName, const FVector& Location, EQuest_NPCInteractionType InteractionType)
{
    FQuest_NPCInteractionData NewInteraction;
    NewInteraction.NPCName = NPCName;
    NewInteraction.NPCLocation = Location;
    NewInteraction.InteractionType = InteractionType;
    NewInteraction.bIsCompleted = false;
    NewInteraction.InteractionRange = 500.0f;
    
    // Set appropriate dialogue based on interaction type
    NewInteraction.DialogueText = GetNPCDialogue(NPCName, InteractionType);
    
    // Set default requirements and rewards based on interaction type
    switch (InteractionType)
    {
        case EQuest_NPCInteractionType::Trade:
            NewInteraction.RequiredItems.Add(TEXT("Stone"));
            NewInteraction.RewardItems.Add(TEXT("Food"));
            break;
        case EQuest_NPCInteractionType::Teaching:
            NewInteraction.RequiredItems.Add(TEXT("Experience"));
            NewInteraction.RewardItems.Add(TEXT("Skill_Crafting"));
            break;
        case EQuest_NPCInteractionType::Escort:
            NewInteraction.RequiredItems.Add(TEXT("Protection"));
            NewInteraction.RewardItems.Add(TEXT("Reputation"));
            break;
        default:
            break;
    }
    
    RegisteredNPCs.Add(NewInteraction);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Registered NPC %s for %s interaction at location %s"), 
           *NPCName, 
           *UEnum::GetValueAsString(InteractionType), 
           *Location.ToString());
}

bool UQuest_NPCInteractionManager::StartNPCInteraction(const FString& NPCName, AActor* PlayerActor)
{
    if (!PlayerActor)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_NPCInteractionManager: Invalid player actor for interaction"));
        return false;
    }
    
    for (FQuest_NPCInteractionData& Interaction : RegisteredNPCs)
    {
        if (Interaction.NPCName == NPCName && !Interaction.bIsCompleted)
        {
            float Distance = FVector::Dist(PlayerActor->GetActorLocation(), Interaction.NPCLocation);
            
            if (Distance <= Interaction.InteractionRange)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Starting interaction with %s - %s"), 
                       *NPCName, *Interaction.DialogueText);
                
                // Increment interaction count
                int32* Count = InteractionCounts.Find(NPCName);
                if (Count)
                {
                    (*Count)++;
                }
                else
                {
                    InteractionCounts.Add(NPCName, 1);
                }
                
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Player too far from %s (Distance: %f, Required: %f)"), 
                       *NPCName, Distance, Interaction.InteractionRange);
                return false;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: NPC %s not found or already completed"), *NPCName);
    return false;
}

void UQuest_NPCInteractionManager::CompleteNPCInteraction(const FString& NPCName)
{
    for (FQuest_NPCInteractionData& Interaction : RegisteredNPCs)
    {
        if (Interaction.NPCName == NPCName)
        {
            Interaction.bIsCompleted = true;
            UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Completed interaction with %s"), *NPCName);
            
            // Check if this completes any tribal quests
            for (FQuest_TribalQuestData& TribalQuest : TribalQuests)
            {
                if (TribalQuest.bQuestActive && TribalQuest.CurrentInteractionIndex < TribalQuest.InteractionChain.Num())
                {
                    if (TribalQuest.InteractionChain[TribalQuest.CurrentInteractionIndex].NPCName == NPCName)
                    {
                        AdvanceTribalQuest(TribalQuest.QuestName);
                    }
                }
            }
            break;
        }
    }
}

TArray<FQuest_NPCInteractionData> UQuest_NPCInteractionManager::GetAvailableInteractions() const
{
    TArray<FQuest_NPCInteractionData> AvailableInteractions;
    
    for (const FQuest_NPCInteractionData& Interaction : RegisteredNPCs)
    {
        if (!Interaction.bIsCompleted)
        {
            AvailableInteractions.Add(Interaction);
        }
    }
    
    return AvailableInteractions;
}

FQuest_NPCInteractionData UQuest_NPCInteractionManager::GetNearestInteraction(const FVector& PlayerLocation, float MaxDistance) const
{
    FQuest_NPCInteractionData NearestInteraction;
    float NearestDistance = MaxDistance;
    bool bFoundInteraction = false;
    
    for (const FQuest_NPCInteractionData& Interaction : RegisteredNPCs)
    {
        if (!Interaction.bIsCompleted)
        {
            float Distance = FVector::Dist(PlayerLocation, Interaction.NPCLocation);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestInteraction = Interaction;
                bFoundInteraction = true;
            }
        }
    }
    
    if (!bFoundInteraction)
    {
        // Return default empty interaction
        NearestInteraction.NPCName = TEXT("No NPC Found");
        NearestInteraction.DialogueText = TEXT("No NPCs nearby.");
    }
    
    return NearestInteraction;
}

void UQuest_NPCInteractionManager::CreateTribalQuest(const FString& QuestName, const FString& TribeName, const TArray<FQuest_NPCInteractionData>& InteractionChain)
{
    FQuest_TribalQuestData NewTribalQuest;
    NewTribalQuest.QuestName = QuestName;
    NewTribalQuest.TribeName = TribeName;
    NewTribalQuest.InteractionChain = InteractionChain;
    NewTribalQuest.CurrentInteractionIndex = 0;
    NewTribalQuest.bQuestActive = false;
    NewTribalQuest.QuestTimeLimit = 3600.0f; // 1 hour
    
    TribalQuests.Add(NewTribalQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Created tribal quest '%s' for tribe '%s' with %d interactions"), 
           *QuestName, *TribeName, InteractionChain.Num());
}

bool UQuest_NPCInteractionManager::StartTribalQuest(const FString& QuestName)
{
    for (FQuest_TribalQuestData& TribalQuest : TribalQuests)
    {
        if (TribalQuest.QuestName == QuestName && !TribalQuest.bQuestActive)
        {
            TribalQuest.bQuestActive = true;
            TribalQuest.CurrentInteractionIndex = 0;
            
            // Start quest timer
            QuestTimers.Add(QuestName, 0.0f);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Started tribal quest '%s'"), *QuestName);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Failed to start tribal quest '%s' - not found or already active"), *QuestName);
    return false;
}

void UQuest_NPCInteractionManager::AdvanceTribalQuest(const FString& QuestName)
{
    for (FQuest_TribalQuestData& TribalQuest : TribalQuests)
    {
        if (TribalQuest.QuestName == QuestName && TribalQuest.bQuestActive)
        {
            TribalQuest.CurrentInteractionIndex++;
            
            if (TribalQuest.CurrentInteractionIndex >= TribalQuest.InteractionChain.Num())
            {
                // Quest completed
                TribalQuest.bQuestActive = false;
                QuestTimers.Remove(QuestName);
                
                UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Completed tribal quest '%s'"), *QuestName);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Advanced tribal quest '%s' to interaction %d/%d"), 
                       *QuestName, TribalQuest.CurrentInteractionIndex + 1, TribalQuest.InteractionChain.Num());
            }
            break;
        }
    }
}

TArray<FQuest_TribalQuestData> UQuest_NPCInteractionManager::GetActiveTribalQuests() const
{
    TArray<FQuest_TribalQuestData> ActiveQuests;
    
    for (const FQuest_TribalQuestData& TribalQuest : TribalQuests)
    {
        if (TribalQuest.bQuestActive)
        {
            ActiveQuests.Add(TribalQuest);
        }
    }
    
    return ActiveQuests;
}

void UQuest_NPCInteractionManager::SyncWithCrowdSystem()
{
    if (CrowdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Syncing with crowd simulation system"));
        
        // This would integrate with the crowd system to get NPC locations and states
        // For now, we'll register some default NPCs based on crowd system data
        SpawnQuestNPCs();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: CrowdManager not found, cannot sync"));
    }
}

void UQuest_NPCInteractionManager::SpawnQuestNPCs()
{
    // Spawn quest-specific NPCs in different biomes
    TArray<FVector> NPCSpawnLocations = {
        FVector(0, 0, 100),           // Savanna center
        FVector(-25000, -22500, 100), // Swamp area
        FVector(-22500, 20000, 100),  // Forest area
        FVector(27500, 0, 100),       // Desert area
        FVector(20000, 25000, 200)    // Mountain area
    };
    
    TArray<FString> NPCNames = {
        TEXT("Tribal_Elder_Krog"),
        TEXT("Swamp_Shaman_Nala"),
        TEXT("Forest_Hunter_Thok"),
        TEXT("Desert_Nomad_Zara"),
        TEXT("Mountain_Guide_Brok")
    };
    
    TArray<EQuest_NPCInteractionType> InteractionTypes = {
        EQuest_NPCInteractionType::Teaching,
        EQuest_NPCInteractionType::Information,
        EQuest_NPCInteractionType::Trade,
        EQuest_NPCInteractionType::Escort,
        EQuest_NPCInteractionType::Alliance
    };
    
    for (int32 i = 0; i < NPCSpawnLocations.Num() && i < NPCNames.Num(); i++)
    {
        RegisterNPCForInteraction(NPCNames[i], NPCSpawnLocations[i], InteractionTypes[i]);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Spawned %d quest NPCs"), NPCSpawnLocations.Num());
}

FString UQuest_NPCInteractionManager::GetNPCDialogue(const FString& NPCName, EQuest_NPCInteractionType InteractionType) const
{
    // Check for custom dialogue first
    const FString* CustomDialogue = DefaultDialogues.Find(InteractionType);
    if (CustomDialogue)
    {
        return *CustomDialogue;
    }
    
    // Return default dialogue based on interaction type
    switch (InteractionType)
    {
        case EQuest_NPCInteractionType::Trade:
            return TEXT("I have resources to trade. What do you offer?");
        case EQuest_NPCInteractionType::Information:
            return TEXT("I know much about these lands. Ask me anything.");
        case EQuest_NPCInteractionType::Escort:
            return TEXT("The path ahead is dangerous. Will you guide me?");
        case EQuest_NPCInteractionType::Rescue:
            return TEXT("Help! I am trapped and need assistance!");
        case EQuest_NPCInteractionType::Teaching:
            return TEXT("I can teach you the old ways, if you are willing to learn.");
        case EQuest_NPCInteractionType::Alliance:
            return TEXT("Our tribes could be stronger together. What say you?");
        case EQuest_NPCInteractionType::Warning:
            return TEXT("Beware! Great danger approaches from the east!");
        default:
            return TEXT("Greetings, traveler.");
    }
}

void UQuest_NPCInteractionManager::SetCustomDialogue(const FString& NPCName, const FString& CustomDialogue)
{
    // Find the NPC and set custom dialogue
    for (FQuest_NPCInteractionData& Interaction : RegisteredNPCs)
    {
        if (Interaction.NPCName == NPCName)
        {
            Interaction.DialogueText = CustomDialogue;
            UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Set custom dialogue for %s: %s"), *NPCName, *CustomDialogue);
            break;
        }
    }
}

void UQuest_NPCInteractionManager::InitializeDefaultInteractions()
{
    // This will be called during initialization to set up default NPCs
    RegisteredNPCs.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Initialized default interactions"));
}

void UQuest_NPCInteractionManager::CreateDefaultTribalQuests()
{
    // Create a sample tribal quest chain
    TArray<FQuest_NPCInteractionData> QuestChain;
    
    FQuest_NPCInteractionData Step1;
    Step1.NPCName = TEXT("Tribal_Elder_Krog");
    Step1.InteractionType = EQuest_NPCInteractionType::Information;
    Step1.DialogueText = TEXT("The ancient hunting grounds hold many secrets.");
    
    FQuest_NPCInteractionData Step2;
    Step2.NPCName = TEXT("Forest_Hunter_Thok");
    Step2.InteractionType = EQuest_NPCInteractionType::Teaching;
    Step2.DialogueText = TEXT("I will teach you to track the great beasts.");
    
    FQuest_NPCInteractionData Step3;
    Step3.NPCName = TEXT("Tribal_Elder_Krog");
    Step3.InteractionType = EQuest_NPCInteractionType::Alliance;
    Step3.DialogueText = TEXT("You have proven yourself worthy. Join our tribe.");
    
    QuestChain.Add(Step1);
    QuestChain.Add(Step2);
    QuestChain.Add(Step3);
    
    CreateTribalQuest(TEXT("Path_of_the_Hunter"), TEXT("Stone_Spear_Tribe"), QuestChain);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Created default tribal quest"));
}

bool UQuest_NPCInteractionManager::ValidateNPCInteraction(const FQuest_NPCInteractionData& InteractionData) const
{
    // Validate that the interaction data is complete and valid
    if (InteractionData.NPCName.IsEmpty())
    {
        return false;
    }
    
    if (InteractionData.DialogueText.IsEmpty())
    {
        return false;
    }
    
    if (InteractionData.InteractionRange <= 0.0f)
    {
        return false;
    }
    
    return true;
}

void UQuest_NPCInteractionManager::UpdateQuestProgress()
{
    // Update quest timers and check for time limits
    for (auto& QuestTimer : QuestTimers)
    {
        QuestTimer.Value += GetWorld()->GetDeltaSeconds();
        
        // Check if quest has exceeded time limit
        for (FQuest_TribalQuestData& TribalQuest : TribalQuests)
        {
            if (TribalQuest.QuestName == QuestTimer.Key && TribalQuest.bQuestActive)
            {
                if (QuestTimer.Value >= TribalQuest.QuestTimeLimit)
                {
                    // Quest failed due to time limit
                    TribalQuest.bQuestActive = false;
                    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Quest '%s' failed due to time limit"), *TribalQuest.QuestName);
                }
            }
        }
    }
}

void UQuest_NPCInteractionManager::SetupDefaultDialogues()
{
    DefaultDialogues.Empty();
    
    DefaultDialogues.Add(EQuest_NPCInteractionType::Trade, TEXT("I have goods to trade. Show me what you have."));
    DefaultDialogues.Add(EQuest_NPCInteractionType::Information, TEXT("I have traveled far and seen much. What would you know?"));
    DefaultDialogues.Add(EQuest_NPCInteractionType::Escort, TEXT("The journey is perilous alone. Will you accompany me?"));
    DefaultDialogues.Add(EQuest_NPCInteractionType::Rescue, TEXT("Please help! I am in great danger!"));
    DefaultDialogues.Add(EQuest_NPCInteractionType::Teaching, TEXT("The old knowledge must be preserved. Are you ready to learn?"));
    DefaultDialogues.Add(EQuest_NPCInteractionType::Alliance, TEXT("Together we are stronger than apart. Will you join us?"));
    DefaultDialogues.Add(EQuest_NPCInteractionType::Warning, TEXT("Danger stalks these lands! Heed my words!"));
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_NPCInteractionManager: Setup default dialogue templates"));
}