#include "Audio_GameFeedbackSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/Engine.h"

UAudio_GameFeedbackSystem::UAudio_GameFeedbackSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5 seconds
    
    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    
    bFeedbackEnabled = true;
    PlayerPawn = nullptr;
    CachedWorld = nullptr;
}

void UAudio_GameFeedbackSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache world reference
    CachedWorld = GetWorld();
    
    // Find player pawn
    if (CachedWorld)
    {
        PlayerPawn = UGameplayStatics::GetPlayerPawn(CachedWorld, 0);
    }
    
    // Initialize feedback configurations
    InitializeFeedbackConfigs();
    
    // Setup audio component
    if (AudioComponent)
    {
        AudioComponent->SetAutoActivate(false);
        AudioComponent->bAllowSpatialization = true;
    }
}

void UAudio_GameFeedbackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bFeedbackEnabled || !CachedWorld)
    {
        return;
    }
    
    // Update cooldowns
    UpdateCooldowns(DeltaTime);
    
    // Check environmental feedback
    CheckEnvironmentalFeedback();
}

void UAudio_GameFeedbackSystem::TriggerFeedback(EAudio_FeedbackType FeedbackType, FVector Location)
{
    if (!bFeedbackEnabled || !CanTriggerFeedback(FeedbackType))
    {
        return;
    }
    
    const FAudio_FeedbackConfig* Config = FeedbackConfigs.Find(FeedbackType);
    if (!Config)
    {
        return;
    }
    
    // Use player location if no location specified
    if (Location == FVector::ZeroVector && PlayerPawn)
    {
        Location = PlayerPawn->GetActorLocation();
    }
    
    // Play the feedback audio
    PlayFeedbackAudio(*Config, Location);
    
    // Set cooldown
    FeedbackCooldowns.Add(FeedbackType, Config->Cooldown);
    
    // Log for debugging
    UE_LOG(LogTemp, Log, TEXT("Audio Feedback Triggered: %s"), 
           *UEnum::GetValueAsString(FeedbackType));
}

void UAudio_GameFeedbackSystem::CheckProximityFeedback(AActor* TargetActor, EAudio_FeedbackType FeedbackType)
{
    if (!bFeedbackEnabled || !PlayerPawn || !TargetActor)
    {
        return;
    }
    
    const FAudio_FeedbackConfig* Config = FeedbackConfigs.Find(FeedbackType);
    if (!Config)
    {
        return;
    }
    
    // Calculate distance
    float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), TargetActor->GetActorLocation());
    
    // Check if within trigger distance
    if (Distance <= Config->TriggerDistance)
    {
        TriggerFeedback(FeedbackType, TargetActor->GetActorLocation());
    }
}

void UAudio_GameFeedbackSystem::CheckSurvivalFeedback(float Health, float Hunger, float Thirst, float Fear)
{
    if (!bFeedbackEnabled)
    {
        return;
    }
    
    // Check critical health
    if (Health < 0.25f)
    {
        TriggerFeedback(EAudio_FeedbackType::LowHealth);
    }
    
    // Check hunger
    if (Hunger < 0.3f)
    {
        TriggerFeedback(EAudio_FeedbackType::LowHunger);
    }
    
    // Check thirst
    if (Thirst < 0.3f)
    {
        TriggerFeedback(EAudio_FeedbackType::LowThirst);
    }
    
    // Check fear level
    if (Fear > 0.7f)
    {
        TriggerFeedback(EAudio_FeedbackType::HighFear);
    }
}

void UAudio_GameFeedbackSystem::CheckEnvironmentalFeedback()
{
    if (!bFeedbackEnabled || !PlayerPawn || !CachedWorld)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check for nearby actors that might trigger audio feedback
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(CachedWorld, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (!Actor || Actor == PlayerPawn)
        {
            continue;
        }
        
        FString ActorName = Actor->GetName().ToLower();
        
        // Check for T-Rex proximity
        if (ActorName.Contains(TEXT("trex")) || ActorName.Contains(TEXT("tyrannosaurus")))
        {
            CheckProximityFeedback(Actor, EAudio_FeedbackType::TRexProximity);
        }
        // Check for Raptor proximity
        else if (ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("velociraptor")))
        {
            CheckProximityFeedback(Actor, EAudio_FeedbackType::RaptorPack);
        }
        // Check for water sources
        else if (ActorName.Contains(TEXT("water")) || ActorName.Contains(TEXT("river")) || ActorName.Contains(TEXT("stream")))
        {
            CheckProximityFeedback(Actor, EAudio_FeedbackType::WaterNearby);
        }
    }
}

void UAudio_GameFeedbackSystem::SetFeedbackConfig(EAudio_FeedbackType FeedbackType, const FAudio_FeedbackConfig& Config)
{
    FeedbackConfigs.Add(FeedbackType, Config);
}

FAudio_FeedbackConfig UAudio_GameFeedbackSystem::GetFeedbackConfig(EAudio_FeedbackType FeedbackType) const
{
    const FAudio_FeedbackConfig* Config = FeedbackConfigs.Find(FeedbackType);
    if (Config)
    {
        return *Config;
    }
    
    return FAudio_FeedbackConfig();
}

void UAudio_GameFeedbackSystem::InitializeFeedbackConfigs()
{
    SetupDefaultConfigs();
}

bool UAudio_GameFeedbackSystem::CanTriggerFeedback(EAudio_FeedbackType FeedbackType) const
{
    const float* Cooldown = FeedbackCooldowns.Find(FeedbackType);
    return !Cooldown || *Cooldown <= 0.0f;
}

void UAudio_GameFeedbackSystem::PlayFeedbackAudio(const FAudio_FeedbackConfig& Config, FVector Location)
{
    if (!AudioComponent || !CachedWorld)
    {
        return;
    }
    
    // Load the sound cue if specified
    USoundCue* SoundCue = Config.AudioCue.LoadSynchronous();
    if (!SoundCue)
    {
        // Use a default beep sound for testing
        UE_LOG(LogTemp, Warning, TEXT("No sound cue specified for feedback, using default"));
        return;
    }
    
    if (Config.bSpatial)
    {
        // Play spatial audio at location
        UGameplayStatics::PlaySoundAtLocation(
            CachedWorld,
            SoundCue,
            Location,
            Config.Volume,
            Config.Pitch
        );
    }
    else
    {
        // Play 2D audio
        UGameplayStatics::PlaySound2D(
            CachedWorld,
            SoundCue,
            Config.Volume,
            Config.Pitch
        );
    }
}

void UAudio_GameFeedbackSystem::UpdateCooldowns(float DeltaTime)
{
    TArray<EAudio_FeedbackType> KeysToRemove;
    
    for (auto& Pair : FeedbackCooldowns)
    {
        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.0f)
        {
            KeysToRemove.Add(Pair.Key);
        }
    }
    
    // Remove expired cooldowns
    for (EAudio_FeedbackType Key : KeysToRemove)
    {
        FeedbackCooldowns.Remove(Key);
    }
}

void UAudio_GameFeedbackSystem::SetupDefaultConfigs()
{
    // T-Rex proximity feedback
    FAudio_FeedbackConfig TRexConfig;
    TRexConfig.FeedbackType = EAudio_FeedbackType::TRexProximity;
    TRexConfig.TriggerDistance = 2000.0f; // 20 meters
    TRexConfig.Volume = 0.8f;
    TRexConfig.Cooldown = 10.0f;
    TRexConfig.bSpatial = true;
    FeedbackConfigs.Add(EAudio_FeedbackType::TRexProximity, TRexConfig);
    
    // Raptor pack feedback
    FAudio_FeedbackConfig RaptorConfig;
    RaptorConfig.FeedbackType = EAudio_FeedbackType::RaptorPack;
    RaptorConfig.TriggerDistance = 1500.0f; // 15 meters
    RaptorConfig.Volume = 0.7f;
    RaptorConfig.Cooldown = 8.0f;
    RaptorConfig.bSpatial = true;
    FeedbackConfigs.Add(EAudio_FeedbackType::RaptorPack, RaptorConfig);
    
    // Low health feedback
    FAudio_FeedbackConfig HealthConfig;
    HealthConfig.FeedbackType = EAudio_FeedbackType::LowHealth;
    HealthConfig.Volume = 0.6f;
    HealthConfig.Cooldown = 15.0f;
    HealthConfig.bSpatial = false;
    FeedbackConfigs.Add(EAudio_FeedbackType::LowHealth, HealthConfig);
    
    // Low hunger feedback
    FAudio_FeedbackConfig HungerConfig;
    HungerConfig.FeedbackType = EAudio_FeedbackType::LowHunger;
    HungerConfig.Volume = 0.5f;
    HungerConfig.Cooldown = 20.0f;
    HungerConfig.bSpatial = false;
    FeedbackConfigs.Add(EAudio_FeedbackType::LowHunger, HungerConfig);
    
    // Low thirst feedback
    FAudio_FeedbackConfig ThirstConfig;
    ThirstConfig.FeedbackType = EAudio_FeedbackType::LowThirst;
    ThirstConfig.Volume = 0.5f;
    ThirstConfig.Cooldown = 18.0f;
    ThirstConfig.bSpatial = false;
    FeedbackConfigs.Add(EAudio_FeedbackType::LowThirst, ThirstConfig);
    
    // High fear feedback
    FAudio_FeedbackConfig FearConfig;
    FearConfig.FeedbackType = EAudio_FeedbackType::HighFear;
    FearConfig.Volume = 0.7f;
    FearConfig.Cooldown = 12.0f;
    FearConfig.bSpatial = false;
    FeedbackConfigs.Add(EAudio_FeedbackType::HighFear, FearConfig);
    
    // Water nearby feedback
    FAudio_FeedbackConfig WaterConfig;
    WaterConfig.FeedbackType = EAudio_FeedbackType::WaterNearby;
    WaterConfig.TriggerDistance = 800.0f; // 8 meters
    WaterConfig.Volume = 0.4f;
    WaterConfig.Cooldown = 25.0f;
    WaterConfig.bSpatial = true;
    FeedbackConfigs.Add(EAudio_FeedbackType::WaterNearby, WaterConfig);
    
    UE_LOG(LogTemp, Log, TEXT("Audio Game Feedback System: Default configurations loaded"));
}