#include "MetaSoundIntegration.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "MetasoundDataReference.h"
#include "MetasoundExecutableOperator.h"
#include "MetasoundFacade.h"
#include "MetasoundGenerator.h"
#include "MetasoundLog.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundPrimitives.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundVertex.h"
#include "UObject/ObjectMacros.h"

DEFINE_LOG_CATEGORY(LogMetaSoundIntegration);

UMetaSoundIntegration::UMetaSoundIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    // Initialize MetaSound parameters
    CurrentTensionLevel = 0.0f;
    CurrentDangerLevel = 0.0f;
    CurrentEmotionalState = EEmotionalState::Neutral;
    
    // Initialize parameter mappings
    InitializeParameterMappings();
}

void UMetaSoundIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize MetaSound system
    InitializeMetaSoundSystem();
    
    // Register for game state updates
    RegisterGameStateCallbacks();
    
    UE_LOG(LogMetaSoundIntegration, Log, TEXT("MetaSound Integration initialized"));
}

void UMetaSoundIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update MetaSound parameters based on game state
    UpdateMetaSoundParameters(DeltaTime);
    
    // Process emotional state transitions
    ProcessEmotionalStateTransitions(DeltaTime);
    
    // Update procedural audio generation
    UpdateProceduralAudio(DeltaTime);
}

void UMetaSoundIntegration::InitializeMetaSoundSystem()
{
    // Create MetaSound generator for adaptive music
    if (AdaptiveMusicMetaSound)
    {
        AdaptiveMusicGenerator = CreateMetaSoundGenerator(AdaptiveMusicMetaSound);
        if (AdaptiveMusicGenerator.IsValid())
        {
            // Set initial parameters
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("TensionLevel"), CurrentTensionLevel);
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("DangerLevel"), CurrentDangerLevel);
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("EmotionalState"), static_cast<float>(CurrentEmotionalState));
            
            UE_LOG(LogMetaSoundIntegration, Log, TEXT("Adaptive music MetaSound generator created"));
        }
    }
    
    // Create MetaSound generator for environmental audio
    if (EnvironmentalAudioMetaSound)
    {
        EnvironmentalAudioGenerator = CreateMetaSoundGenerator(EnvironmentalAudioMetaSound);
        if (EnvironmentalAudioGenerator.IsValid())
        {
            // Set environmental parameters
            SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("TimeOfDay"), 0.5f);
            SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("WeatherIntensity"), 0.3f);
            SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("BiomeType"), 1.0f); // Forest
            
            UE_LOG(LogMetaSoundIntegration, Log, TEXT("Environmental audio MetaSound generator created"));
        }
    }
    
    // Create MetaSound generator for creature vocalizations
    if (CreatureVocalizationMetaSound)
    {
        CreatureVocalizationGenerator = CreateMetaSoundGenerator(CreatureVocalizationMetaSound);
        if (CreatureVocalizationGenerator.IsValid())
        {
            UE_LOG(LogMetaSoundIntegration, Log, TEXT("Creature vocalization MetaSound generator created"));
        }
    }
}

TSharedPtr<Metasound::FMetasoundGenerator> UMetaSoundIntegration::CreateMetaSoundGenerator(UMetaSoundSource* MetaSoundSource)
{
    if (!MetaSoundSource)
    {
        UE_LOG(LogMetaSoundIntegration, Warning, TEXT("MetaSoundSource is null"));
        return nullptr;
    }
    
    // Create generator settings
    Metasound::FMetasoundGeneratorInitParams InitParams;
    InitParams.MetaSoundSource = MetaSoundSource;
    InitParams.SampleRate = 48000.0f;
    InitParams.NumChannels = 2;
    
    // Create the generator
    TSharedPtr<Metasound::FMetasoundGenerator> Generator = Metasound::FMetasoundGenerator::CreateGenerator(InitParams);
    
    if (!Generator.IsValid())
    {
        UE_LOG(LogMetaSoundIntegration, Error, TEXT("Failed to create MetaSound generator"));
        return nullptr;
    }
    
    return Generator;
}

void UMetaSoundIntegration::SetMetaSoundParameter(TSharedPtr<Metasound::FMetasoundGenerator> Generator, const FString& ParameterName, float Value)
{
    if (!Generator.IsValid())
    {
        return;
    }
    
    // Convert parameter name to MetaSound format
    Metasound::FVertexName VertexName(*ParameterName);
    
    // Set the parameter value
    Generator->SetInputValue(VertexName, Value);
    
    UE_LOG(LogMetaSoundIntegration, Verbose, TEXT("Set MetaSound parameter %s to %f"), *ParameterName, Value);
}

void UMetaSoundIntegration::UpdateMetaSoundParameters(float DeltaTime)
{
    // Update tension level based on nearby threats
    float NewTensionLevel = CalculateTensionLevel();
    if (FMath::Abs(NewTensionLevel - CurrentTensionLevel) > 0.01f)
    {
        CurrentTensionLevel = FMath::FInterpTo(CurrentTensionLevel, NewTensionLevel, DeltaTime, TensionTransitionSpeed);
        
        // Update MetaSound parameters
        if (AdaptiveMusicGenerator.IsValid())
        {
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("TensionLevel"), CurrentTensionLevel);
        }
    }
    
    // Update danger level based on combat state
    float NewDangerLevel = CalculateDangerLevel();
    if (FMath::Abs(NewDangerLevel - CurrentDangerLevel) > 0.01f)
    {
        CurrentDangerLevel = FMath::FInterpTo(CurrentDangerLevel, NewDangerLevel, DeltaTime, DangerTransitionSpeed);
        
        // Update MetaSound parameters
        if (AdaptiveMusicGenerator.IsValid())
        {
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("DangerLevel"), CurrentDangerLevel);
        }
    }
    
    // Update emotional state
    EEmotionalState NewEmotionalState = CalculateEmotionalState();
    if (NewEmotionalState != CurrentEmotionalState)
    {
        CurrentEmotionalState = NewEmotionalState;
        
        // Update MetaSound parameters
        if (AdaptiveMusicGenerator.IsValid())
        {
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("EmotionalState"), static_cast<float>(CurrentEmotionalState));
        }
        
        UE_LOG(LogMetaSoundIntegration, Log, TEXT("Emotional state changed to: %d"), static_cast<int32>(CurrentEmotionalState));
    }
}

float UMetaSoundIntegration::CalculateTensionLevel()
{
    float TensionLevel = 0.0f;
    
    // Get player pawn
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return TensionLevel;
    }
    
    // Check for nearby threats
    TArray<AActor*> NearbyActors;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        PlayerPawn->GetActorLocation(),
        ThreatDetectionRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        NearbyActors
    );
    
    // Calculate tension based on threat proximity and type
    for (AActor* Actor : NearbyActors)
    {
        if (Actor->ActorHasTag("Predator"))
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
            float ThreatWeight = FMath::Clamp(1.0f - (Distance / ThreatDetectionRadius), 0.0f, 1.0f);
            TensionLevel += ThreatWeight * 0.8f; // High tension for predators
        }
        else if (Actor->ActorHasTag("Herbivore"))
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
            float ThreatWeight = FMath::Clamp(1.0f - (Distance / ThreatDetectionRadius), 0.0f, 1.0f);
            TensionLevel += ThreatWeight * 0.2f; // Low tension for herbivores
        }
    }
    
    return FMath::Clamp(TensionLevel, 0.0f, 1.0f);
}

float UMetaSoundIntegration::CalculateDangerLevel()
{
    float DangerLevel = 0.0f;
    
    // Get player pawn
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return DangerLevel;
    }
    
    // Check player health
    if (UHealthComponent* HealthComp = PlayerPawn->FindComponentByClass<UHealthComponent>())
    {
        float HealthPercentage = HealthComp->GetHealthPercentage();
        DangerLevel += (1.0f - HealthPercentage) * 0.5f;
    }
    
    // Check for active combat
    if (UCombatComponent* CombatComp = PlayerPawn->FindComponentByClass<UCombatComponent>())
    {
        if (CombatComp->IsInCombat())
        {
            DangerLevel += 0.7f;
        }
    }
    
    // Check environmental hazards
    if (PlayerPawn->ActorHasTag("InDanger"))
    {
        DangerLevel += 0.3f;
    }
    
    return FMath::Clamp(DangerLevel, 0.0f, 1.0f);
}

EEmotionalState UMetaSoundIntegration::CalculateEmotionalState()
{
    // Determine emotional state based on tension and danger levels
    if (CurrentDangerLevel > 0.7f)
    {
        return EEmotionalState::Fear;
    }
    else if (CurrentTensionLevel > 0.6f)
    {
        return EEmotionalState::Anxiety;
    }
    else if (CurrentTensionLevel > 0.3f)
    {
        return EEmotionalState::Caution;
    }
    else if (CurrentTensionLevel < 0.1f && CurrentDangerLevel < 0.1f)
    {
        return EEmotionalState::Wonder;
    }
    else
    {
        return EEmotionalState::Neutral;
    }
}

void UMetaSoundIntegration::ProcessEmotionalStateTransitions(float DeltaTime)
{
    // Handle emotional state transition effects
    static EEmotionalState PreviousState = EEmotionalState::Neutral;
    
    if (CurrentEmotionalState != PreviousState)
    {
        // Trigger transition effects
        OnEmotionalStateChanged.Broadcast(PreviousState, CurrentEmotionalState);
        
        // Play transition sounds
        PlayEmotionalTransitionSound(PreviousState, CurrentEmotionalState);
        
        PreviousState = CurrentEmotionalState;
    }
}

void UMetaSoundIntegration::PlayEmotionalTransitionSound(EEmotionalState FromState, EEmotionalState ToState)
{
    // Find appropriate transition sound
    FString TransitionKey = FString::Printf(TEXT("%d_to_%d"), static_cast<int32>(FromState), static_cast<int32>(ToState));
    
    if (USoundBase** TransitionSound = EmotionalTransitionSounds.Find(TransitionKey))
    {
        if (*TransitionSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), *TransitionSound, GetOwner()->GetActorLocation());
            UE_LOG(LogMetaSoundIntegration, Log, TEXT("Playing emotional transition sound: %s"), *TransitionKey);
        }
    }
}

void UMetaSoundIntegration::UpdateProceduralAudio(float DeltaTime)
{
    // Update environmental audio based on time of day and weather
    if (EnvironmentalAudioGenerator.IsValid())
    {
        float TimeOfDay = GetWorld()->GetTimeSeconds() / 86400.0f; // Normalize to 0-1
        TimeOfDay = FMath::Fmod(TimeOfDay, 1.0f);
        
        SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("TimeOfDay"), TimeOfDay);
        
        // Update weather intensity (this would come from weather system)
        float WeatherIntensity = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.1f) * 0.5f + 0.5f;
        SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("WeatherIntensity"), WeatherIntensity);
    }
    
    // Update creature vocalizations based on nearby creatures
    if (CreatureVocalizationGenerator.IsValid())
    {
        UpdateCreatureVocalizations(DeltaTime);
    }
}

void UMetaSoundIntegration::UpdateCreatureVocalizations(float DeltaTime)
{
    // Get nearby creatures
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    TArray<AActor*> NearbyCreatures;
    UKismetSystemLibrary::SphereOverlapActors(
        GetWorld(),
        PlayerPawn->GetActorLocation(),
        VocalizationDetectionRadius,
        TArray<TEnumAsByte<EObjectTypeQuery>>(),
        nullptr,
        TArray<AActor*>(),
        NearbyCreatures
    );
    
    // Count creature types
    int32 HerbivoreCount = 0;
    int32 PredatorCount = 0;
    
    for (AActor* Actor : NearbyCreatures)
    {
        if (Actor->ActorHasTag("Herbivore"))
        {
            HerbivoreCount++;
        }
        else if (Actor->ActorHasTag("Predator"))
        {
            PredatorCount++;
        }
    }
    
    // Update MetaSound parameters
    SetMetaSoundParameter(CreatureVocalizationGenerator, TEXT("HerbivoreCount"), static_cast<float>(HerbivoreCount));
    SetMetaSoundParameter(CreatureVocalizationGenerator, TEXT("PredatorCount"), static_cast<float>(PredatorCount));
    SetMetaSoundParameter(CreatureVocalizationGenerator, TEXT("PlayerProximity"), FMath::Clamp(NearbyCreatures.Num() / 10.0f, 0.0f, 1.0f));
}

void UMetaSoundIntegration::InitializeParameterMappings()
{
    // Initialize parameter mapping for different MetaSound assets
    ParameterMappings.Add(TEXT("AdaptiveMusic"), {
        TEXT("TensionLevel"),
        TEXT("DangerLevel"),
        TEXT("EmotionalState"),
        TEXT("TimeOfDay"),
        TEXT("BiomeType")
    });
    
    ParameterMappings.Add(TEXT("EnvironmentalAudio"), {
        TEXT("TimeOfDay"),
        TEXT("WeatherIntensity"),
        TEXT("BiomeType"),
        TEXT("PlayerActivity"),
        TEXT("CreatureDensity")
    });
    
    ParameterMappings.Add(TEXT("CreatureVocalizations"), {
        TEXT("HerbivoreCount"),
        TEXT("PredatorCount"),
        TEXT("PlayerProximity"),
        TEXT("ThreatLevel"),
        TEXT("TimeOfDay")
    });
}

void UMetaSoundIntegration::RegisterGameStateCallbacks()
{
    // Register for game state updates
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        // Register callbacks for various game events
        // This would integrate with the game's event system
        UE_LOG(LogMetaSoundIntegration, Log, TEXT("Game state callbacks registered"));
    }
}

void UMetaSoundIntegration::SetEmotionalState(EEmotionalState NewState)
{
    if (CurrentEmotionalState != NewState)
    {
        EEmotionalState PreviousState = CurrentEmotionalState;
        CurrentEmotionalState = NewState;
        
        // Update MetaSound parameters
        if (AdaptiveMusicGenerator.IsValid())
        {
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("EmotionalState"), static_cast<float>(CurrentEmotionalState));
        }
        
        // Broadcast state change
        OnEmotionalStateChanged.Broadcast(PreviousState, CurrentEmotionalState);
        
        UE_LOG(LogMetaSoundIntegration, Log, TEXT("Emotional state manually set to: %d"), static_cast<int32>(CurrentEmotionalState));
    }
}

void UMetaSoundIntegration::TriggerAudioEvent(const FString& EventName, const FVector& Location, float Intensity)
{
    // Trigger specific audio events through MetaSound
    if (EventName == TEXT("DinosaurRoar"))
    {
        if (CreatureVocalizationGenerator.IsValid())
        {
            SetMetaSoundParameter(CreatureVocalizationGenerator, TEXT("TriggerRoar"), 1.0f);
            SetMetaSoundParameter(CreatureVocalizationGenerator, TEXT("RoarIntensity"), Intensity);
        }
    }
    else if (EventName == TEXT("FootstepHeavy"))
    {
        if (EnvironmentalAudioGenerator.IsValid())
        {
            SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("TriggerFootstep"), 1.0f);
            SetMetaSoundParameter(EnvironmentalAudioGenerator, TEXT("FootstepIntensity"), Intensity);
        }
    }
    else if (EventName == TEXT("CrystalResonance"))
    {
        // Special event for the mystical crystal
        if (AdaptiveMusicGenerator.IsValid())
        {
            SetMetaSoundParameter(AdaptiveMusicGenerator, TEXT("CrystalResonance"), Intensity);
        }
    }
    
    UE_LOG(LogMetaSoundIntegration, Log, TEXT("Audio event triggered: %s at intensity %f"), *EventName, Intensity);
}

float UMetaSoundIntegration::GetCurrentTensionLevel() const
{
    return CurrentTensionLevel;
}

float UMetaSoundIntegration::GetCurrentDangerLevel() const
{
    return CurrentDangerLevel;
}

EEmotionalState UMetaSoundIntegration::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}