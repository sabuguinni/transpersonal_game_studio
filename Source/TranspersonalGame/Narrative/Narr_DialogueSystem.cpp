#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"

ANarr_DialogueSystem::ANarr_DialogueSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    AudioComponent->SetupAttachment(RootComponent);
    AudioComponent->bAutoActivate = false;

    // Initialize settings
    DialogueCooldown = 15.0f;
    ContextUpdateInterval = 5.0f;
    bAutoTriggerDialogue = true;
    MaxDialogueDistance = 5000.0f;
    bDialogueOnCooldown = false;

    // Initialize context
    CurrentContext = FNarr_NarrativeContext();
}

void ANarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize dialogue database
    InitializeDialogueDatabase();

    // Start context update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ContextUpdateTimer,
            this,
            &ANarr_DialogueSystem::OnContextUpdate,
            ContextUpdateInterval,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue System initialized with %d dialogue entries"), DialogueDatabase.Num());
}

void ANarr_DialogueSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-trigger dialogue based on context changes
    if (bAutoTriggerDialogue && !bDialogueOnCooldown && IsPlayerNearby())
    {
        // Check for context-based dialogue triggers
        if (CurrentContext.bInEmergency)
        {
            TriggerDialogue(ENarr_DialogueType::Emergency);
        }
        else if (CurrentContext.ThreatLevel == EEng_ThreatLevel::Dangerous || CurrentContext.ThreatLevel == EEng_ThreatLevel::Deadly)
        {
            TriggerDialogue(ENarr_DialogueType::Warning);
        }
        else if (CurrentContext.NearbyDinosaurs.Num() > 0)
        {
            EEng_DinosaurSpecies NearestDinosaur = CurrentContext.NearbyDinosaurs[0];
            TriggerDialogue(ENarr_DialogueType::Observation, NearestDinosaur);
        }
    }
}

void ANarr_DialogueSystem::InitializeDialogueDatabase()
{
    DialogueDatabase.Empty();

    // Discovery Dialogues
    FNarr_DialogueEntry DiscoveryEntry;
    DiscoveryEntry.DialogueText = "Registo de campo, dia 89. Descobri um ninho de Triceratops abandonado. Os ovos parecem intactos, mas não há sinais dos pais.";
    DiscoveryEntry.Speaker = ENarr_SpeakerRole::FieldPaleontologist;
    DiscoveryEntry.DialogueType = ENarr_DialogueType::Discovery;
    DiscoveryEntry.Duration = 17.0f;
    DiscoveryEntry.RelevantDinosaur = EEng_DinosaurSpecies::Triceratops;
    DiscoveryEntry.RelevantBiome = EEng_BiomeType::Forest;
    DialogueDatabase.Add(DiscoveryEntry);

    // Exploration Dialogues
    FNarr_DialogueEntry ExplorationEntry;
    ExplorationEntry.DialogueText = "Atenção, explorador! Descobri vestígios de uma antiga migração de Brachiosaurus nesta região. As pegadas indicam que se dirigiam para norte.";
    ExplorationEntry.Speaker = ENarr_SpeakerRole::ExplorationGuide;
    ExplorationEntry.DialogueType = ENarr_DialogueType::Observation;
    ExplorationEntry.Duration = 19.0f;
    ExplorationEntry.RelevantDinosaur = EEng_DinosaurSpecies::Brachiosaurus;
    ExplorationEntry.RelevantBiome = EEng_BiomeType::Savanna;
    DialogueDatabase.Add(ExplorationEntry);

    // Scientific Discovery
    FNarr_DialogueEntry ScientificEntry;
    ScientificEntry.DialogueText = "Descoberta fascinante! Encontrámos fósseis de Triceratops nesta área. As marcas nos ossos sugerem confronto com um grande predador.";
    ScientificEntry.Speaker = ENarr_SpeakerRole::SeniorPaleontologist;
    ScientificEntry.DialogueType = ENarr_DialogueType::Discovery;
    ScientificEntry.Duration = 19.0f;
    ScientificEntry.RelevantDinosaur = EEng_DinosaurSpecies::Triceratops;
    ScientificEntry.RelevantBiome = EEng_BiomeType::Desert;
    DialogueDatabase.Add(ScientificEntry);

    // Safety Warnings
    FNarr_DialogueEntry SafetyEntry;
    SafetyEntry.DialogueText = "Alerta de perigo! Dinossauro carnívoro detectado na vossa proximidade. Procurem abrigo imediatamente e evitem movimentos bruscos.";
    SafetyEntry.Speaker = ENarr_SpeakerRole::SafetyCoordinator;
    SafetyEntry.DialogueType = ENarr_DialogueType::Warning;
    SafetyEntry.Duration = 16.0f;
    SafetyEntry.RelevantDinosaur = EEng_DinosaurSpecies::TRex;
    SafetyEntry.RelevantBiome = EEng_BiomeType::Forest;
    DialogueDatabase.Add(SafetyEntry);

    // T-Rex Encounter
    FNarr_DialogueEntry TRexEntry;
    TRexEntry.DialogueText = "Tyrannosaurus Rex avistado! Este é o predador apex do Cretáceo. Mantenham distância segura e documentem o comportamento.";
    TRexEntry.Speaker = ENarr_SpeakerRole::ResearchDirector;
    TRexEntry.DialogueType = ENarr_DialogueType::Warning;
    TRexEntry.Duration = 15.0f;
    TRexEntry.RelevantDinosaur = EEng_DinosaurSpecies::TRex;
    DialogueDatabase.Add(TRexEntry);

    // Raptor Pack Warning
    FNarr_DialogueEntry RaptorEntry;
    RaptorEntry.DialogueText = "Atenção! Matilha de Velociraptors detectada. Estes predadores caçam em grupo com estratégias coordenadas. Extrema cautela necessária.";
    RaptorEntry.Speaker = ENarr_SpeakerRole::SurvivalExpert;
    RaptorEntry.DialogueType = ENarr_DialogueType::Warning;
    RaptorEntry.Duration = 18.0f;
    RaptorEntry.RelevantDinosaur = EEng_DinosaurSpecies::Raptor;
    DialogueDatabase.Add(RaptorEntry);

    // Herbivore Observation
    FNarr_DialogueEntry HerbivoreEntry;
    HerbivoreEntry.DialogueText = "Brachiosaurus pacífico avistado. Estes gigantes herbívoros são geralmente inofensivos, mas o seu tamanho pode ser perigoso se assustados.";
    HerbivoreEntry.Speaker = ENarr_SpeakerRole::FieldPaleontologist;
    HerbivoreEntry.DialogueType = ENarr_DialogueType::Observation;
    HerbivoreEntry.Duration = 16.0f;
    HerbivoreEntry.RelevantDinosaur = EEng_DinosaurSpecies::Brachiosaurus;
    DialogueDatabase.Add(HerbivoreEntry);

    // Biome-specific entries
    FNarr_DialogueEntry SwampEntry;
    SwampEntry.DialogueText = "Entraram numa zona pantanosa. Cuidado com a lama profunda e predadores aquáticos. A visibilidade é limitada aqui.";
    SwampEntry.Speaker = ENarr_SpeakerRole::ExplorationGuide;
    SwampEntry.DialogueType = ENarr_DialogueType::Tutorial;
    SwampEntry.Duration = 14.0f;
    SwampEntry.RelevantBiome = EEng_BiomeType::Swamp;
    DialogueDatabase.Add(SwampEntry);

    FNarr_DialogueEntry MountainEntry;
    MountainEntry.DialogueText = "Região montanhosa à frente. As temperaturas baixas e terreno rochoso apresentam novos desafios. Preparem-se adequadamente.";
    MountainEntry.Speaker = ENarr_SpeakerRole::SurvivalExpert;
    MountainEntry.DialogueType = ENarr_DialogueType::Tutorial;
    MountainEntry.Duration = 15.0f;
    MountainEntry.RelevantBiome = EEng_BiomeType::SnowyMountain;
    DialogueDatabase.Add(MountainEntry);

    UE_LOG(LogTemp, Warning, TEXT("Dialogue database initialized with %d entries"), DialogueDatabase.Num());
}

void ANarr_DialogueSystem::TriggerDialogue(ENarr_DialogueType DialogueType, EEng_DinosaurSpecies RelevantDinosaur)
{
    if (bDialogueOnCooldown)
    {
        return;
    }

    FNarr_DialogueEntry SelectedDialogue = GetRelevantDialogue(DialogueType, CurrentContext.CurrentBiome, RelevantDinosaur);
    
    if (!SelectedDialogue.DialogueText.IsEmpty())
    {
        PlayDialogue(SelectedDialogue);
        
        // Start cooldown
        bDialogueOnCooldown = true;
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                DialogueCooldownTimer,
                this,
                &ANarr_DialogueSystem::OnDialogueCooldownEnd,
                DialogueCooldown,
                false
            );
        }
    }
}

FNarr_DialogueEntry ANarr_DialogueSystem::GetRelevantDialogue(ENarr_DialogueType DialogueType, EEng_BiomeType Biome, EEng_DinosaurSpecies Dinosaur)
{
    // First, try to find exact match (type + biome + dinosaur)
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueType == DialogueType && 
            Entry.RelevantBiome == Biome && 
            Entry.RelevantDinosaur == Dinosaur)
        {
            return Entry;
        }
    }

    // Second, try to find match by type and dinosaur
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueType == DialogueType && Entry.RelevantDinosaur == Dinosaur)
        {
            return Entry;
        }
    }

    // Third, try to find match by type and biome
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueType == DialogueType && Entry.RelevantBiome == Biome)
        {
            return Entry;
        }
    }

    // Finally, try to find any match by type
    for (const FNarr_DialogueEntry& Entry : DialogueDatabase)
    {
        if (Entry.DialogueType == DialogueType)
        {
            return Entry;
        }
    }

    // Return empty entry if no match found
    return FNarr_DialogueEntry();
}

void ANarr_DialogueSystem::PlayDialogue(const FNarr_DialogueEntry& DialogueEntry)
{
    UE_LOG(LogTemp, Warning, TEXT("Playing dialogue: %s"), *DialogueEntry.DialogueText);
    
    // Display text to screen for debugging
    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("[%s]: %s"), 
            *UEnum::GetValueAsString(DialogueEntry.Speaker), 
            *DialogueEntry.DialogueText);
        GEngine->AddOnScreenDebugMessage(-1, DialogueEntry.Duration, FColor::Cyan, DisplayText);
    }

    // TODO: Integrate with audio system when audio URLs are available
    // For now, just log the dialogue
}

void ANarr_DialogueSystem::UpdateNarrativeContext()
{
    CurrentContext.CurrentBiome = DetectCurrentBiome();
    CurrentContext.NearbyDinosaurs = DetectNearbyDinosaurs();
    CurrentContext.ThreatLevel = CalculateThreatLevel();
    
    // Update time and weather (simplified for now)
    CurrentContext.TimeOfDay = EEng_TimeOfDay::Morning;
    CurrentContext.Weather = EEng_WeatherType::Clear;
}

EEng_BiomeType ANarr_DialogueSystem::DetectCurrentBiome()
{
    // Get player location
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                
                // Determine biome based on coordinates (using brain memory coordinates)
                if (PlayerLocation.X >= -77500 && PlayerLocation.X <= -25000 && 
                    PlayerLocation.Y >= -76500 && PlayerLocation.Y <= -15000)
                {
                    return EEng_BiomeType::Swamp;
                }
                else if (PlayerLocation.X >= -77500 && PlayerLocation.X <= -15000 && 
                         PlayerLocation.Y >= 15000 && PlayerLocation.Y <= 76500)
                {
                    return EEng_BiomeType::Forest;
                }
                else if (PlayerLocation.X >= -20000 && PlayerLocation.X <= 20000 && 
                         PlayerLocation.Y >= -20000 && PlayerLocation.Y <= 20000)
                {
                    return EEng_BiomeType::Savanna;
                }
                else if (PlayerLocation.X >= 25000 && PlayerLocation.X <= 79500 && 
                         PlayerLocation.Y >= -30000 && PlayerLocation.Y <= 30000)
                {
                    return EEng_BiomeType::Desert;
                }
                else if (PlayerLocation.X >= 15000 && PlayerLocation.X <= 79500 && 
                         PlayerLocation.Y >= 20000 && PlayerLocation.Y <= 76500)
                {
                    return EEng_BiomeType::SnowyMountain;
                }
            }
        }
    }
    
    return EEng_BiomeType::Savanna; // Default
}

TArray<EEng_DinosaurSpecies> ANarr_DialogueSystem::DetectNearbyDinosaurs()
{
    TArray<EEng_DinosaurSpecies> NearbyDinosaurs;
    
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            FString ActorName = Actor->GetName().ToLower();
            
            // Check distance to player
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                if (APawn* PlayerPawn = PC->GetPawn())
                {
                    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerPawn->GetActorLocation());
                    if (Distance <= MaxDialogueDistance)
                    {
                        // Identify dinosaur species by name
                        if (ActorName.Contains("trex"))
                        {
                            NearbyDinosaurs.AddUnique(EEng_DinosaurSpecies::TRex);
                        }
                        else if (ActorName.Contains("raptor"))
                        {
                            NearbyDinosaurs.AddUnique(EEng_DinosaurSpecies::Raptor);
                        }
                        else if (ActorName.Contains("brachio"))
                        {
                            NearbyDinosaurs.AddUnique(EEng_DinosaurSpecies::Brachiosaurus);
                        }
                        else if (ActorName.Contains("tricera"))
                        {
                            NearbyDinosaurs.AddUnique(EEng_DinosaurSpecies::Triceratops);
                        }
                        else if (ActorName.Contains("stego"))
                        {
                            NearbyDinosaurs.AddUnique(EEng_DinosaurSpecies::Stegosaurus);
                        }
                    }
                }
            }
        }
    }
    
    return NearbyDinosaurs;
}

EEng_ThreatLevel ANarr_DialogueSystem::CalculateThreatLevel()
{
    EEng_ThreatLevel ThreatLevel = EEng_ThreatLevel::Safe;
    
    for (EEng_DinosaurSpecies Species : CurrentContext.NearbyDinosaurs)
    {
        switch (Species)
        {
            case EEng_DinosaurSpecies::TRex:
                ThreatLevel = EEng_ThreatLevel::Deadly;
                break;
            case EEng_DinosaurSpecies::Raptor:
                if (ThreatLevel < EEng_ThreatLevel::Dangerous)
                    ThreatLevel = EEng_ThreatLevel::Dangerous;
                break;
            case EEng_DinosaurSpecies::Triceratops:
                if (ThreatLevel < EEng_ThreatLevel::Cautious)
                    ThreatLevel = EEng_ThreatLevel::Cautious;
                break;
            default:
                break;
        }
    }
    
    return ThreatLevel;
}

void ANarr_DialogueSystem::OnEmergencyTriggered(const FString& EmergencyType)
{
    CurrentContext.bInEmergency = true;
    TriggerDialogue(ENarr_DialogueType::Emergency);
}

void ANarr_DialogueSystem::OnMissionCompleted(const FString& MissionType)
{
    CurrentContext.bInEmergency = false;
    TriggerDialogue(ENarr_DialogueType::Achievement);
}

void ANarr_DialogueSystem::AddCustomDialogue(const FNarr_DialogueEntry& NewDialogue)
{
    DialogueDatabase.Add(NewDialogue);
}

void ANarr_DialogueSystem::ClearDialogueDatabase()
{
    DialogueDatabase.Empty();
}

void ANarr_DialogueSystem::OnContextUpdate()
{
    UpdateNarrativeContext();
}

void ANarr_DialogueSystem::OnDialogueCooldownEnd()
{
    bDialogueOnCooldown = false;
}

bool ANarr_DialogueSystem::IsPlayerNearby()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
                return Distance <= MaxDialogueDistance;
            }
        }
    }
    return false;
}

FString ANarr_DialogueSystem::GetBiomeName(EEng_BiomeType BiomeType)
{
    return UEnum::GetValueAsString(BiomeType);
}

FString ANarr_DialogueSystem::GetDinosaurName(EEng_DinosaurSpecies Species)
{
    return UEnum::GetValueAsString(Species);
}