#include "Audio_MetaSoundController.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "MetasoundSource.h"

UAudio_MetaSoundController::UAudio_MetaSoundController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default ambience zones
    AmbienceZones.SetNum(6);
    
    // Forest ambience
    AmbienceZones[0].AmbienceType = EAudio_AmbienceType::Forest;
    AmbienceZones[0].VolumeMultiplier = 0.8f;
    AmbienceZones[0].FadeDistance = 2500.0f;
    
    // Savanna ambience  
    AmbienceZones[1].AmbienceType = EAudio_AmbienceType::Savanna;
    AmbienceZones[1].VolumeMultiplier = 0.9f;
    AmbienceZones[1].FadeDistance = 3000.0f;
    
    // River ambience
    AmbienceZones[2].AmbienceType = EAudio_AmbienceType::River;
    AmbienceZones[2].VolumeMultiplier = 1.0f;
    AmbienceZones[2].FadeDistance = 1500.0f;
    
    // Mountain ambience
    AmbienceZones[3].AmbienceType = EAudio_AmbienceType::Mountain;
    AmbienceZones[3].VolumeMultiplier = 0.7f;
    AmbienceZones[3].FadeDistance = 4000.0f;
    
    // Cave ambience
    AmbienceZones[4].AmbienceType = EAudio_AmbienceType::Cave;
    AmbienceZones[4].VolumeMultiplier = 0.6f;
    AmbienceZones[4].FadeDistance = 800.0f;
    
    // Swamp ambience
    AmbienceZones[5].AmbienceType = EAudio_AmbienceType::Swamp;
    AmbienceZones[5].VolumeMultiplier = 0.9f;
    AmbienceZones[5].FadeDistance = 2000.0f;
    
    // Set defaults
    GlobalVolumeMultiplier = 1.0f;
    AmbienceFadeSpeed = 2.0f;
    FootstepVolumeRange = 1500.0f;
    DangerProximityThreshold = 1000.0f;
    TensionVolumeMultiplier = 1.5f;
    
    CurrentAmbienceType = EAudio_AmbienceType::Forest;
    CurrentAmbienceVolume = 1.0f;
    TargetAmbienceVolume = 1.0f;
    CurrentAmbienceComponent = nullptr;
}

void UAudio_MetaSoundController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start with forest ambience
    PlayAmbienceForZone(EAudio_AmbienceType::Forest, GetOwner()->GetActorLocation());
}

void UAudio_MetaSoundController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update ambience volume smoothly
    UpdateAmbienceVolume(DeltaTime);
    
    // Clean up finished audio components
    CleanupFinishedAudioComponents();
    
    // Check for dynamic audio mixing based on nearby threats
    if (UWorld* World = GetWorld())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Check for nearby dinosaurs to increase tension
            TArray<AActor*> FoundActors;
            UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);
            
            float ClosestDinosaurDistance = DangerProximityThreshold + 1000.0f;
            for (AActor* Actor : FoundActors)
            {
                if (Actor != PlayerPawn && Actor->GetName().Contains(TEXT("Dino")))
                {
                    float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                    if (Distance < ClosestDinosaurDistance)
                    {
                        ClosestDinosaurDistance = Distance;
                    }
                }
            }
            
            // Calculate danger level
            float DangerLevel = 0.0f;
            if (ClosestDinosaurDistance < DangerProximityThreshold)
            {
                DangerLevel = 1.0f - (ClosestDinosaurDistance / DangerProximityThreshold);
            }
            
            UpdateDynamicMixing(DangerLevel, 1.0f); // Assume full stamina for now
        }
    }
}

void UAudio_MetaSoundController::PlayAmbienceForZone(EAudio_AmbienceType ZoneType, FVector PlayerLocation)
{
    FAudio_AmbienceZone* Zone = FindAmbienceZoneByType(ZoneType);
    if (!Zone || !Zone->MetaSoundAsset.IsValid())
    {
        return;
    }
    
    // Stop current ambience if different zone
    if (CurrentAmbienceType != ZoneType && CurrentAmbienceComponent)
    {
        CurrentAmbienceComponent->FadeOut(1.0f, 0.0f);
        CurrentAmbienceComponent = nullptr;
    }
    
    // Create new ambience component if needed
    if (!CurrentAmbienceComponent && GetOwner())
    {
        CurrentAmbienceComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(),
            Zone->MetaSoundAsset.Get(),
            PlayerLocation,
            FRotator::ZeroRotator,
            Zone->VolumeMultiplier * GlobalVolumeMultiplier
        );
        
        if (CurrentAmbienceComponent)
        {
            CurrentAmbienceComponent->bAutoDestroy = false;
            CurrentAmbienceType = ZoneType;
            TargetAmbienceVolume = Zone->VolumeMultiplier;
        }
    }
}

void UAudio_MetaSoundController::TriggerFootstepAudio(FVector FootstepLocation, bool bIsPlayerFootstep)
{
    UMetaSoundSource* FootstepSound = bIsPlayerFootstep ? 
        PlayerFootstepMetaSound.Get() : DinosaurFootstepMetaSound.Get();
    
    if (!FootstepSound)
    {
        return;
    }
    
    // Calculate volume based on distance to player
    if (UWorld* World = GetWorld())
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), FootstepLocation);
            float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / FootstepVolumeRange), 0.1f, 1.0f);
            
            UAudioComponent* FootstepComponent = UGameplayStatics::SpawnSoundAtLocation(
                World,
                FootstepSound,
                FootstepLocation,
                FRotator::ZeroRotator,
                VolumeMultiplier * GlobalVolumeMultiplier
            );
            
            if (FootstepComponent)
            {
                FootstepComponents.Add(FootstepComponent);
            }
        }
    }
}

void UAudio_MetaSoundController::UpdateDynamicMixing(float DangerLevel, float StaminaLevel)
{
    // Increase ambience volume when in danger
    float DynamicVolumeMultiplier = 1.0f + (DangerLevel * (TensionVolumeMultiplier - 1.0f));
    
    if (CurrentAmbienceComponent)
    {
        float NewVolume = TargetAmbienceVolume * DynamicVolumeMultiplier * GlobalVolumeMultiplier;
        CurrentAmbienceComponent->SetVolumeMultiplier(NewVolume);
    }
}

void UAudio_MetaSoundController::SetGlobalAudioVolume(float NewVolume)
{
    GlobalVolumeMultiplier = FMath::Clamp(NewVolume, 0.0f, 2.0f);
    
    // Update current ambience volume
    if (CurrentAmbienceComponent)
    {
        CurrentAmbienceComponent->SetVolumeMultiplier(
            CurrentAmbienceVolume * GlobalVolumeMultiplier
        );
    }
}

void UAudio_MetaSoundController::UpdateAmbienceVolume(float DeltaTime)
{
    if (FMath::Abs(CurrentAmbienceVolume - TargetAmbienceVolume) > 0.01f)
    {
        CurrentAmbienceVolume = FMath::FInterpTo(
            CurrentAmbienceVolume,
            TargetAmbienceVolume,
            DeltaTime,
            AmbienceFadeSpeed
        );
        
        if (CurrentAmbienceComponent)
        {
            CurrentAmbienceComponent->SetVolumeMultiplier(
                CurrentAmbienceVolume * GlobalVolumeMultiplier
            );
        }
    }
}

void UAudio_MetaSoundController::CleanupFinishedAudioComponents()
{
    FootstepComponents.RemoveAll([](UAudioComponent* Component)
    {
        return !Component || !Component->IsPlaying();
    });
}

FAudio_AmbienceZone* UAudio_MetaSoundController::FindAmbienceZoneByType(EAudio_AmbienceType ZoneType)
{
    for (FAudio_AmbienceZone& Zone : AmbienceZones)
    {
        if (Zone.AmbienceType == ZoneType)
        {
            return &Zone;
        }
    }
    return nullptr;
}