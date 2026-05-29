#include "Audio_EffectManager.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundCue.h"

AAudio_EffectManager::AAudio_EffectManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio component
    AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
    RootComponent = AudioComponent;

    // Initialize effect database with default values
    FAudio_EffectData TRexEffect;
    TRexEffect.EffectType = EAudio_EffectType::TRexProximity;
    TRexEffect.Volume = 0.8f;
    TRexEffect.Pitch = 0.9f;
    EffectDatabase.Add(TRexEffect);

    FAudio_EffectData DamageEffect;
    DamageEffect.EffectType = EAudio_EffectType::DamageFlash;
    DamageEffect.Volume = 1.0f;
    DamageEffect.Pitch = 1.2f;
    EffectDatabase.Add(DamageEffect);

    FAudio_EffectData FootstepEffect;
    FootstepEffect.EffectType = EAudio_EffectType::FootstepDust;
    FootstepEffect.Volume = 0.6f;
    FootstepEffect.Pitch = 1.0f;
    EffectDatabase.Add(FootstepEffect);

    FAudio_EffectData AmbientEffect;
    AmbientEffect.EffectType = EAudio_EffectType::AmbientNature;
    AmbientEffect.Volume = 0.4f;
    AmbientEffect.Pitch = 1.0f;
    EffectDatabase.Add(AmbientEffect);
}

void AAudio_EffectManager::BeginPlay()
{
    Super::BeginPlay();

    // Get player controller and pawn references
    PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        PlayerPawn = PlayerController->GetPawn();
    }

    // Start ambient nature sounds
    PlayEffect(EAudio_EffectType::AmbientNature);
}

void AAudio_EffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check T-Rex proximity every frame
    CheckTRexProximity();

    // Handle damage flash timer
    if (bDamageFlashActive)
    {
        DamageFlashTimer -= DeltaTime;
        if (DamageFlashTimer <= 0.0f)
        {
            bDamageFlashActive = false;
            // Reset screen overlay (would be handled by UI system)
        }
    }
}

void AAudio_EffectManager::PlayEffect(EAudio_EffectType EffectType, FVector Location)
{
    // Find effect data
    FAudio_EffectData* EffectData = EffectDatabase.FindByPredicate([EffectType](const FAudio_EffectData& Data)
    {
        return Data.EffectType == EffectType;
    });

    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Audio effect type not found in database"));
        return;
    }

    // Check if effect is already playing
    if (ActiveEffects.Contains(EffectType))
    {
        UAudioComponent* ExistingComponent = ActiveEffects[EffectType];
        if (ExistingComponent && ExistingComponent->IsPlaying())
        {
            return; // Already playing
        }
    }

    // Create new audio component for this effect
    UAudioComponent* NewAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
        this,
        EffectData->SoundCue.LoadSynchronous(),
        Location.IsZero() ? GetActorLocation() : Location,
        FRotator::ZeroRotator,
        EffectData->Volume,
        EffectData->Pitch
    );

    if (NewAudioComponent)
    {
        ActiveEffects.Add(EffectType, NewAudioComponent);
        
        // Apply fade in
        if (EffectData->FadeInTime > 0.0f)
        {
            NewAudioComponent->FadeIn(EffectData->FadeInTime, EffectData->Volume);
        }

        UE_LOG(LogTemp, Log, TEXT("Playing audio effect: %d"), (int32)EffectType);
    }
}

void AAudio_EffectManager::StopEffect(EAudio_EffectType EffectType)
{
    if (ActiveEffects.Contains(EffectType))
    {
        UAudioComponent* AudioComp = ActiveEffects[EffectType];
        if (AudioComp && AudioComp->IsPlaying())
        {
            // Find fade out time
            FAudio_EffectData* EffectData = EffectDatabase.FindByPredicate([EffectType](const FAudio_EffectData& Data)
            {
                return Data.EffectType == EffectType;
            });

            float FadeOutTime = EffectData ? EffectData->FadeOutTime : 0.5f;
            AudioComp->FadeOut(FadeOutTime, 0.0f);
        }
        ActiveEffects.Remove(EffectType);
    }
}

void AAudio_EffectManager::CheckTRexProximity()
{
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find all T-Rex actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    bool bTRexNearby = false;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("TRex")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance <= TRexDetectionRadius)
            {
                bTRexNearby = true;
                
                // Calculate screen shake intensity based on distance
                float ShakeIntensity = FMath::Clamp(1.0f - (Distance / TRexDetectionRadius), 0.1f, 1.0f);
                
                // Trigger screen shake (would need camera shake class)
                if (PlayerController)
                {
                    // PlayerController->ClientStartCameraShake(CameraShakeClass, ShakeIntensity);
                }
                
                break;
            }
        }
    }

    // Play or stop T-Rex proximity audio
    static bool bWasNearby = false;
    if (bTRexNearby && !bWasNearby)
    {
        PlayEffect(EAudio_EffectType::TRexProximity, PlayerLocation);
    }
    else if (!bTRexNearby && bWasNearby)
    {
        StopEffect(EAudio_EffectType::TRexProximity);
    }
    bWasNearby = bTRexNearby;
}

void AAudio_EffectManager::TriggerDamageFlash()
{
    if (!bDamageFlashActive)
    {
        bDamageFlashActive = true;
        DamageFlashTimer = DamageFlashDuration;
        
        // Play damage audio
        PlayEffect(EAudio_EffectType::DamageFlash);
        
        // Trigger screen flash effect (would be handled by UI/HUD system)
        UE_LOG(LogTemp, Log, TEXT("Damage flash triggered - Duration: %f"), DamageFlashDuration);
    }
}

void AAudio_EffectManager::CreateFootstepDust(FVector Location, float Intensity)
{
    // Play footstep audio
    PlayEffect(EAudio_EffectType::FootstepDust, Location);
    
    // Create dust particle effect (would spawn Niagara system)
    UE_LOG(LogTemp, Log, TEXT("Footstep dust created at location: %s, Intensity: %f"), 
           *Location.ToString(), Intensity);
}