#include "TranspersonalGameMode.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

ATranspersonalGameMode::ATranspersonalGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
}

void ATranspersonalGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTranspersonalSystems();
    
    UE_LOG(LogTemp, Log, TEXT("Transpersonal Game Mode: Systems Initialized"));
}

void ATranspersonalGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Debug consciousness system if enabled
    if (bDebugConsciousnessSystem && PlayerConsciousnessComponent.IsValid())
    {
        EConsciousnessState CurrentState = PlayerConsciousnessComponent->GetCurrentState();
        float CurrentIntensity = PlayerConsciousnessComponent->GetCurrentIntensity();
        
        if (GEngine)
        {
            FString DebugText = FString::Printf(
                TEXT("Consciousness Debug - State: %s, Intensity: %.1f%%, Global Multiplier: %.2f"),
                *UEnum::GetValueAsString(CurrentState),
                CurrentIntensity,
                GlobalConsciousnessMultiplier
            );
            GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, DebugText);
        }
    }
}

void ATranspersonalGameMode::InitializeTranspersonalSystems()
{
    if (!bEnableTranspersonalFeatures)
    {
        UE_LOG(LogTemp, Warning, TEXT("Transpersonal features disabled in game mode"));
        return;
    }

    InitializeAudioSystem();
    InitializeNarrativeSystem();
    SetupConsciousnessCallbacks();
}

void ATranspersonalGameMode::InitializeAudioSystem()
{
    if (UWorld* World = GetWorld())
    {
        AudioManager = World->GetGameInstance()->GetSubsystem<UTranspersonalAudioManager>();
        
        if (AudioManager)
        {
            UE_LOG(LogTemp, Log, TEXT("Transpersonal Audio Manager initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize Transpersonal Audio Manager"));
        }
    }
}

void ATranspersonalGameMode::InitializeNarrativeSystem()
{
    if (UWorld* World = GetWorld())
    {
        NarrativeManager = World->GetSubsystem<UNarrativeStateManager>();
        
        if (NarrativeManager)
        {
            UE_LOG(LogTemp, Log, TEXT("Narrative State Manager initialized"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to initialize Narrative State Manager"));
        }
    }
}

void ATranspersonalGameMode::SetupConsciousnessCallbacks()
{
    // This will be called when a player registers their consciousness component
    UE_LOG(LogTemp, Log, TEXT("Consciousness callback system ready"));
}

void ATranspersonalGameMode::RegisterPlayerConsciousness(UConsciousnessComponent* ConsciousnessComp)
{
    if (!ConsciousnessComp)
    {
        UE_LOG(LogTemp, Error, TEXT("Attempted to register null consciousness component"));
        return;
    }

    PlayerConsciousnessComponent = ConsciousnessComp;
    
    // Bind to consciousness change events
    ConsciousnessComp->OnConsciousnessChanged.AddDynamic(this, &ATranspersonalGameMode::OnPlayerConsciousnessChanged);
    
    UE_LOG(LogTemp, Log, TEXT("Player consciousness component registered and callbacks bound"));
}

void ATranspersonalGameMode::TriggerGlobalConsciousnessEvent(const FGameplayTag& EventTag, float Intensity)
{
    if (!PlayerConsciousnessComponent.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("No player consciousness component to trigger event on"));
        return;
    }

    // Apply global multiplier to intensity
    float AdjustedIntensity = Intensity * GlobalConsciousnessMultiplier;
    
    // Trigger the consciousness shift
    PlayerConsciousnessComponent->TriggerConsciousnessShift(EventTag);
    
    UE_LOG(LogTemp, Log, TEXT("Global consciousness event triggered: %s (Intensity: %.1f)"), 
           *EventTag.ToString(), AdjustedIntensity);
}

void ATranspersonalGameMode::OnPlayerConsciousnessChanged(EConsciousnessState NewState, float Intensity)
{
    // Update audio system
    if (AudioManager)
    {
        AudioManager->UpdateAudioForConsciousnessState(NewState, Intensity);
    }
    
    // Update narrative system
    if (NarrativeManager)
    {
        NarrativeManager->TriggerConsciousnessBasedNarrative(NewState, Intensity);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Game Mode: Consciousness changed to %s (%.1f%%) - Systems updated"), 
           *UEnum::GetValueAsString(NewState), Intensity);
}