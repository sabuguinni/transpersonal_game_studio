#include "Narr_DialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"

// UNarr_DialogueComponent Implementation
UNarr_DialogueComponent::UNarr_DialogueComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    GlobalCooldown = 5.0f;
    LastDialogueTime = 0.0f;
}

void UNarr_DialogueComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with subsystem
    if (UNarr_DialogueSubsystem* DialogueSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueSubsystem>())
    {
        DialogueSubsystem->RegisterDialogueComponent(this);
    }
    
    // Initialize default cooldowns
    LastTriggerTimes.Add(ENarr_DialogueTrigger::LowHealth, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::Hunger, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::Thirst, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::DinosaurNearby, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::CombatWarning, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::Discovery, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::QuestComplete, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::FireDiscovered, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::ShelterBuilt, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::NightFalling, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::WeatherChange, 0.0f);
    LastTriggerTimes.Add(ENarr_DialogueTrigger::TerritoryEntered, 0.0f);
}

void UNarr_DialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update dialogue timing
    if (GetWorld())
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        // Could add automatic context checking here for ambient dialogue
    }
}

void UNarr_DialogueComponent::TriggerDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context)
{
    if (!CanTriggerDialogue(TriggerType))
    {
        return;
    }
    
    FNarr_DialogueEntry* BestDialogue = FindBestDialogue(TriggerType, Context);
    if (BestDialogue)
    {
        // Log dialogue trigger
        UE_LOG(LogTemp, Warning, TEXT("Dialogue Triggered: %s - %s"), 
               *BestDialogue->CharacterName, *BestDialogue->DialogueText);
        
        // Play audio if available
        PlayDialogueAudio(*BestDialogue);
        
        // Update cooldowns
        float CurrentTime = GetWorld()->GetTimeSeconds();
        LastTriggerTimes.Add(TriggerType, CurrentTime);
        LastDialogueTime = CurrentTime;
        
        // Broadcast to UI/HUD systems
        if (GEngine)
        {
            FString DisplayText = FString::Printf(TEXT("%s: %s"), 
                                                *BestDialogue->CharacterName, 
                                                *BestDialogue->DialogueText);
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, DisplayText);
        }
    }
}

void UNarr_DialogueComponent::AddDialogueEntry(const FNarr_DialogueEntry& Entry)
{
    DialogueEntries.Add(Entry);
}

bool UNarr_DialogueComponent::CanTriggerDialogue(ENarr_DialogueTrigger TriggerType) const
{
    if (!GetWorld())
    {
        return false;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check global cooldown
    if (CurrentTime - LastDialogueTime < GlobalCooldown)
    {
        return false;
    }
    
    // Check specific trigger cooldown
    if (const float* LastTriggerTime = LastTriggerTimes.Find(TriggerType))
    {
        // Find the cooldown time for this trigger type
        float CooldownTime = 30.0f; // Default
        for (const FNarr_DialogueEntry& Entry : DialogueEntries)
        {
            if (Entry.TriggerType == TriggerType)
            {
                CooldownTime = Entry.CooldownTime;
                break;
            }
        }
        
        if (CurrentTime - *LastTriggerTime < CooldownTime)
        {
            return false;
        }
    }
    
    return true;
}

void UNarr_DialogueComponent::SetDialogueCooldown(ENarr_DialogueTrigger TriggerType, float CooldownTime)
{
    for (FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.TriggerType == TriggerType)
        {
            Entry.CooldownTime = CooldownTime;
        }
    }
}

FNarr_DialogueEntry* UNarr_DialogueComponent::FindBestDialogue(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context)
{
    FNarr_DialogueEntry* BestEntry = nullptr;
    float HighestPriority = -1.0f;
    
    for (FNarr_DialogueEntry& Entry : DialogueEntries)
    {
        if (Entry.TriggerType == TriggerType && EvaluateDialogueConditions(Entry, Context))
        {
            if (Entry.Priority > HighestPriority)
            {
                HighestPriority = Entry.Priority;
                BestEntry = &Entry;
            }
        }
    }
    
    return BestEntry;
}

bool UNarr_DialogueComponent::EvaluateDialogueConditions(const FNarr_DialogueEntry& Entry, const FNarr_DialogueContext& Context) const
{
    // Evaluate context-specific conditions
    switch (Entry.TriggerType)
    {
        case ENarr_DialogueTrigger::LowHealth:
            return Context.PlayerHealth < 30.0f;
            
        case ENarr_DialogueTrigger::Hunger:
            return Context.PlayerHunger < 25.0f;
            
        case ENarr_DialogueTrigger::Thirst:
            return Context.PlayerThirst < 25.0f;
            
        case ENarr_DialogueTrigger::DinosaurNearby:
            return Context.bDinosaurNearby;
            
        case ENarr_DialogueTrigger::CombatWarning:
            return Context.bInCombat || Context.bDinosaurNearby;
            
        case ENarr_DialogueTrigger::NightFalling:
            return Context.TimeOfDay > 18.0f || Context.TimeOfDay < 6.0f;
            
        case ENarr_DialogueTrigger::WeatherChange:
            return Context.CurrentWeather != EWeatherType::Clear;
            
        default:
            return true; // Default conditions met
    }
}

void UNarr_DialogueComponent::PlayDialogueAudio(const FNarr_DialogueEntry& Entry)
{
    if (!Entry.AudioPath.IsEmpty())
    {
        // Load and play audio asset
        USoundBase* DialogueSound = LoadObject<USoundBase>(nullptr, *Entry.AudioPath);
        if (DialogueSound && GetWorld())
        {
            UGameplayStatics::PlaySound2D(GetWorld(), DialogueSound);
        }
    }
}

// UNarr_DialogueSubsystem Implementation
void UNarr_DialogueSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    MaxSimultaneousDialogues = 3.0f;
    ActiveDialogueCount = 0;
    
    InitializeDefaultDialogues();
    LoadDialoguesFromConfig();
}

void UNarr_DialogueSubsystem::Deinitialize()
{
    RegisteredComponents.Empty();
    GlobalDialogueDatabase.Empty();
    
    Super::Deinitialize();
}

void UNarr_DialogueSubsystem::RegisterDialogueComponent(UNarr_DialogueComponent* Component)
{
    if (Component && !RegisteredComponents.Contains(Component))
    {
        RegisteredComponents.Add(Component);
        
        // Add global dialogues to this component
        for (const FNarr_DialogueEntry& Entry : GlobalDialogueDatabase)
        {
            Component->AddDialogueEntry(Entry);
        }
    }
}

void UNarr_DialogueSubsystem::UnregisterDialogueComponent(UNarr_DialogueComponent* Component)
{
    RegisteredComponents.Remove(Component);
}

void UNarr_DialogueSubsystem::BroadcastDialogueTrigger(ENarr_DialogueTrigger TriggerType, const FNarr_DialogueContext& Context)
{
    if (ActiveDialogueCount >= MaxSimultaneousDialogues)
    {
        return;
    }
    
    // Find the best component to handle this dialogue
    UNarr_DialogueComponent* BestComponent = nullptr;
    for (UNarr_DialogueComponent* Component : RegisteredComponents)
    {
        if (Component && Component->CanTriggerDialogue(TriggerType))
        {
            BestComponent = Component;
            break;
        }
    }
    
    if (BestComponent)
    {
        ActiveDialogueCount++;
        BestComponent->TriggerDialogue(TriggerType, Context);
        
        // Reset counter after delay
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            ActiveDialogueCount = FMath::Max(0, ActiveDialogueCount - 1);
        }, 5.0f, false);
    }
}

void UNarr_DialogueSubsystem::LoadDialogueDatabase()
{
    // Load from external data source (JSON, CSV, etc.)
    // For now, use hardcoded entries
    InitializeDefaultDialogues();
}

void UNarr_DialogueSubsystem::AddGlobalDialogue(const FNarr_DialogueEntry& Entry)
{
    GlobalDialogueDatabase.Add(Entry);
    
    // Add to all registered components
    for (UNarr_DialogueComponent* Component : RegisteredComponents)
    {
        if (Component)
        {
            Component->AddDialogueEntry(Entry);
        }
    }
}

void UNarr_DialogueSubsystem::InitializeDefaultDialogues()
{
    // Survival dialogues
    FNarr_DialogueEntry LowHealthEntry;
    LowHealthEntry.DialogueText = "Your wounds run deep, hunter. Seek shelter and tend to your injuries before they worsen.";
    LowHealthEntry.CharacterName = "Survival Instinct";
    LowHealthEntry.TriggerType = ENarr_DialogueTrigger::LowHealth;
    LowHealthEntry.Priority = 5.0f;
    LowHealthEntry.CooldownTime = 60.0f;
    GlobalDialogueDatabase.Add(LowHealthEntry);
    
    FNarr_DialogueEntry HungerEntry;
    HungerEntry.DialogueText = "The gnawing hunger weakens your spirit. Hunt for meat or gather berries before you collapse.";
    HungerEntry.CharacterName = "Survival Instinct";
    HungerEntry.TriggerType = ENarr_DialogueTrigger::Hunger;
    HungerEntry.Priority = 4.0f;
    HungerEntry.CooldownTime = 45.0f;
    GlobalDialogueDatabase.Add(HungerEntry);
    
    FNarr_DialogueEntry ThirstEntry;
    ThirstEntry.DialogueText = "Your throat burns with thirst. Find fresh water or risk death in these harsh lands.";
    ThirstEntry.CharacterName = "Survival Instinct";
    ThirstEntry.TriggerType = ENarr_DialogueTrigger::Thirst;
    ThirstEntry.Priority = 5.0f;
    ThirstEntry.CooldownTime = 45.0f;
    GlobalDialogueDatabase.Add(ThirstEntry);
    
    // Danger dialogues
    FNarr_DialogueEntry DinosaurEntry;
    DinosaurEntry.DialogueText = "A great beast stalks nearby. Move carefully and avoid sudden movements.";
    DinosaurEntry.CharacterName = "Tribal Elder";
    DinosaurEntry.TriggerType = ENarr_DialogueTrigger::DinosaurNearby;
    DinosaurEntry.Priority = 6.0f;
    DinosaurEntry.CooldownTime = 30.0f;
    GlobalDialogueDatabase.Add(DinosaurEntry);
    
    FNarr_DialogueEntry CombatEntry;
    CombatEntry.DialogueText = "Steel yourself for battle! Strike fast and true, or become prey!";
    CombatEntry.CharacterName = "War Chief";
    CombatEntry.TriggerType = ENarr_DialogueTrigger::CombatWarning;
    CombatEntry.Priority = 7.0f;
    CombatEntry.CooldownTime = 20.0f;
    GlobalDialogueDatabase.Add(CombatEntry);
    
    // Discovery dialogues
    FNarr_DialogueEntry FireEntry;
    FireEntry.DialogueText = "The sacred flame burns bright! This knowledge will keep the darkness at bay.";
    FireEntry.CharacterName = "Fire Keeper";
    FireEntry.TriggerType = ENarr_DialogueTrigger::FireDiscovered;
    FireEntry.Priority = 3.0f;
    FireEntry.CooldownTime = 120.0f;
    FireEntry.bIsRepeatable = false;
    GlobalDialogueDatabase.Add(FireEntry);
    
    FNarr_DialogueEntry ShelterEntry;
    ShelterEntry.DialogueText = "A sturdy shelter protects against the elements. Well built, survivor.";
    ShelterEntry.CharacterName = "Master Builder";
    ShelterEntry.TriggerType = ENarr_DialogueTrigger::ShelterBuilt;
    ShelterEntry.Priority = 3.0f;
    ShelterEntry.CooldownTime = 60.0f;
    GlobalDialogueDatabase.Add(ShelterEntry);
    
    // Environmental dialogues
    FNarr_DialogueEntry NightEntry;
    NightEntry.DialogueText = "Darkness falls upon the land. Predators hunt in the shadows. Stay near the fire.";
    NightEntry.CharacterName = "Night Watcher";
    NightEntry.TriggerType = ENarr_DialogueTrigger::NightFalling;
    NightEntry.Priority = 4.0f;
    NightEntry.CooldownTime = 300.0f; // 5 minutes
    GlobalDialogueDatabase.Add(NightEntry);
}

void UNarr_DialogueSubsystem::LoadDialoguesFromConfig()
{
    // TODO: Load from external configuration files
    // This would parse JSON/CSV files with dialogue data
}

// ANarr_DialogueTrigger Implementation
ANarr_DialogueTrigger::ANarr_DialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create trigger sphere
    TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
    RootComponent = TriggerSphere;
    TriggerSphere->SetSphereRadius(500.0f);
    TriggerSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // Bind overlap events
    TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &ANarr_DialogueTrigger::OnTriggerEnter);
    
    // Default settings
    TriggerType = ENarr_DialogueTrigger::Discovery;
    bAutoTrigger = true;
    TriggerRadius = 500.0f;
    bRequireLineOfSight = false;
    
    // Initialize default dialogue
    DialogueToTrigger.DialogueText = "You have discovered something interesting...";
    DialogueToTrigger.CharacterName = "Explorer";
    DialogueToTrigger.TriggerType = ENarr_DialogueTrigger::Discovery;
    DialogueToTrigger.Priority = 2.0f;
    DialogueToTrigger.CooldownTime = 30.0f;
}

void ANarr_DialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    
    // Set trigger radius
    if (TriggerSphere)
    {
        TriggerSphere->SetSphereRadius(TriggerRadius);
    }
}

void ANarr_DialogueTrigger::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Could add proximity-based auto-triggering logic here
}

void ANarr_DialogueTrigger::ActivateTrigger(AActor* TriggeringActor)
{
    if (!CheckTriggerConditions(TriggeringActor))
    {
        return;
    }
    
    FNarr_DialogueContext Context = BuildDialogueContext(TriggeringActor);
    
    // Broadcast to subsystem
    if (UNarr_DialogueSubsystem* DialogueSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UNarr_DialogueSubsystem>())
    {
        DialogueSubsystem->BroadcastDialogueTrigger(TriggerType, Context);
    }
    
    // Fire blueprint event
    OnDialogueTriggered.Broadcast(DialogueToTrigger, Context);
}

void ANarr_DialogueTrigger::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (bAutoTrigger && OtherActor && OtherActor->IsA<APawn>())
    {
        ActivateTrigger(OtherActor);
    }
}

bool ANarr_DialogueTrigger::CheckTriggerConditions(AActor* TriggeringActor)
{
    if (!TriggeringActor)
    {
        return false;
    }
    
    // Check line of sight if required
    if (bRequireLineOfSight)
    {
        FHitResult HitResult;
        FVector Start = GetActorLocation();
        FVector End = TriggeringActor->GetActorLocation();
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Visibility
        );
        
        if (bHit && HitResult.GetActor() != TriggeringActor)
        {
            return false; // Line of sight blocked
        }
    }
    
    return true;
}

FNarr_DialogueContext ANarr_DialogueTrigger::BuildDialogueContext(AActor* TriggeringActor)
{
    FNarr_DialogueContext Context;
    
    // Get player state information
    if (APawn* PlayerPawn = Cast<APawn>(TriggeringActor))
    {
        // TODO: Get actual player stats from character component
        Context.PlayerHealth = 75.0f; // Placeholder
        Context.PlayerHunger = 60.0f;  // Placeholder
        Context.PlayerThirst = 80.0f;  // Placeholder
    }
    
    // Set environmental context
    Context.CurrentBiome = "Savana"; // TODO: Get from biome system
    Context.TimeOfDay = 12.0f; // TODO: Get from day/night cycle
    Context.CurrentWeather = EWeatherType::Clear; // TODO: Get from weather system
    
    // Check for nearby dinosaurs
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (Actor && Actor->GetName().Contains("Dinosaur"))
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance < 2000.0f) // Within 20 meters
            {
                Context.bDinosaurNearby = true;
                break;
            }
        }
    }
    
    return Context;
}