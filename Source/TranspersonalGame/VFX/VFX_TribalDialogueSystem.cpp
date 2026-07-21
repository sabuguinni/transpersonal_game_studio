#include "VFX_TribalDialogueSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"

UVFX_TribalDialogueSystem::UVFX_TribalDialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize VFX settings
    DialogueBubbleLifetime = 3.0f;
    SoundWaveMaxDistance = 1000.0f;
    AmbientEffectIntensity = 1.0f;
    bAutoStartAmbientEffects = true;

    // Initialize state
    bDialogueBubbleActive = false;
    bAmbientEffectsActive = false;
    CurrentDialogueTimer = 0.0f;
    LastSoundWaveTime = 0.0f;

    // Initialize components
    DialogueBubbleEffect = nullptr;
    SoundWaveEffect = nullptr;
    AmbientDustEffect = nullptr;
}

void UVFX_TribalDialogueSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVFXComponents();
    
    if (bAutoStartAmbientEffects)
    {
        StartAmbientTribalEffects();
    }
}

void UVFX_TribalDialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateDialogueBubbleTimer(DeltaTime);
    CleanupExpiredEffects();
}

void UVFX_TribalDialogueSystem::ShowDialogueBubble(const FString& DialogueText, float Duration)
{
    if (!DialogueBubbleEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_TribalDialogueSystem: DialogueBubbleEffect not initialized"));
        return;
    }

    // Hide existing bubble if active
    if (bDialogueBubbleActive)
    {
        HideDialogueBubble();
    }

    // Start dialogue bubble effect
    DialogueBubbleEffect->SetVisibility(true);
    DialogueBubbleEffect->Activate();
    
    // Set dialogue parameters
    DialogueBubbleLifetime = Duration;
    CurrentDialogueTimer = Duration;
    bDialogueBubbleActive = true;

    UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Showing dialogue bubble for %f seconds: %s"), Duration, *DialogueText);
}

void UVFX_TribalDialogueSystem::HideDialogueBubble()
{
    if (DialogueBubbleEffect)
    {
        DialogueBubbleEffect->SetVisibility(false);
        DialogueBubbleEffect->Deactivate();
    }
    
    bDialogueBubbleActive = false;
    CurrentDialogueTimer = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Hiding dialogue bubble"));
}

void UVFX_TribalDialogueSystem::CreateSoundWaveEffect(const FVector& SourceLocation, float Intensity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Cooldown check
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastSoundWaveTime < SoundWaveCooldown)
    {
        return;
    }
    LastSoundWaveTime = CurrentTime;

    // Create sound wave visualization at source location
    if (SoundWaveEffect)
    {
        SoundWaveEffect->SetWorldLocation(SourceLocation);
        SoundWaveEffect->SetFloatParameter(TEXT("Intensity"), Intensity);
        SoundWaveEffect->SetVisibility(true);
        SoundWaveEffect->Activate();

        UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Created sound wave effect at %s with intensity %f"), 
               *SourceLocation.ToString(), Intensity);
    }
}

void UVFX_TribalDialogueSystem::TriggerAudioScreenShake(float Magnitude)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get player controller for screen shake
    APlayerController* PlayerController = World->GetFirstPlayerController();
    if (!PlayerController)
    {
        return;
    }

    // Create simple screen shake effect
    // Note: In a full implementation, this would use a custom camera shake class
    UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Triggering audio screen shake with magnitude %f"), Magnitude);
    
    // For now, log the screen shake - would need custom camera shake blueprint in full implementation
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
            FString::Printf(TEXT("Audio Screen Shake: %f"), Magnitude));
    }
}

void UVFX_TribalDialogueSystem::StartAmbientTribalEffects()
{
    if (!AmbientDustEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_TribalDialogueSystem: AmbientDustEffect not initialized"));
        return;
    }

    AmbientDustEffect->SetFloatParameter(TEXT("Intensity"), AmbientEffectIntensity);
    AmbientDustEffect->SetVisibility(true);
    AmbientDustEffect->Activate();
    
    bAmbientEffectsActive = true;

    UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Started ambient tribal effects"));
}

void UVFX_TribalDialogueSystem::StopAmbientTribalEffects()
{
    if (AmbientDustEffect)
    {
        AmbientDustEffect->SetVisibility(false);
        AmbientDustEffect->Deactivate();
    }
    
    bAmbientEffectsActive = false;

    UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Stopped ambient tribal effects"));
}

void UVFX_TribalDialogueSystem::InitializeVFXComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Create Niagara components for VFX effects
    DialogueBubbleEffect = NewObject<UNiagaraComponent>(Owner);
    if (DialogueBubbleEffect)
    {
        DialogueBubbleEffect->SetupAttachment(Owner->GetRootComponent());
        DialogueBubbleEffect->SetRelativeLocation(FVector(0, 0, 200)); // Above actor
        DialogueBubbleEffect->SetVisibility(false);
    }

    SoundWaveEffect = NewObject<UNiagaraComponent>(Owner);
    if (SoundWaveEffect)
    {
        SoundWaveEffect->SetupAttachment(Owner->GetRootComponent());
        SoundWaveEffect->SetVisibility(false);
    }

    AmbientDustEffect = NewObject<UNiagaraComponent>(Owner);
    if (AmbientDustEffect)
    {
        AmbientDustEffect->SetupAttachment(Owner->GetRootComponent());
        AmbientDustEffect->SetRelativeLocation(FVector(0, 0, 50));
        AmbientDustEffect->SetVisibility(false);
    }

    UE_LOG(LogTemp, Log, TEXT("VFX_TribalDialogueSystem: Initialized VFX components"));
}

void UVFX_TribalDialogueSystem::UpdateDialogueBubbleTimer(float DeltaTime)
{
    if (bDialogueBubbleActive)
    {
        CurrentDialogueTimer -= DeltaTime;
        
        if (CurrentDialogueTimer <= 0.0f)
        {
            HideDialogueBubble();
        }
    }
}

void UVFX_TribalDialogueSystem::CleanupExpiredEffects()
{
    // Clean up expired sound wave effects
    for (int32 i = ActiveSoundWaveEffects.Num() - 1; i >= 0; --i)
    {
        UNiagaraComponent* Effect = ActiveSoundWaveEffects[i];
        if (!Effect || !Effect->IsActive())
        {
            ActiveSoundWaveEffects.RemoveAt(i);
        }
    }
}