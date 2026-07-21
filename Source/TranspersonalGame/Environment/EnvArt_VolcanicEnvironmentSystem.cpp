#include "EnvArt_VolcanicEnvironmentSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "DrawDebugHelpers.h"

UEnvArt_VolcanicEnvironmentSystem::UEnvArt_VolcanicEnvironmentSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second for performance

    // Initialize default values
    VolcanicActivity = EEnvArt_VolcanicActivity::Dormant;
    VolcanicZoneRadius = 5000.0f;
    VolcanicCenterLocation = FVector::ZeroVector;
    MaxVolcanicProps = 200;
    CurrentVolcanicPropCount = 0;
    LastUpdateTime = 0.0f;
    LODDistance = 3000.0f;
    UpdateFrequency = 2.0f;

    // Initialize component references
    CurrentAshEffect = nullptr;
    CurrentLavaEffect = nullptr;
    CurrentVolcanicAudio = nullptr;

    // Initialize particle systems to null (will be set in Blueprint or by designer)
    AshParticleSystem = nullptr;
    LavaGlowParticleSystem = nullptr;
    SteamVentParticleSystem = nullptr;

    // Initialize sound cues to null
    VolcanicRumbleSound = nullptr;
    LavaBubbleSound = nullptr;
    SteamHissSound = nullptr;

    // Initialize material parameter collection to null
    VolcanicMaterialParams = nullptr;
}

void UEnvArt_VolcanicEnvironmentSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize volcanic center location if not set
    if (VolcanicCenterLocation.IsZero())
    {
        VolcanicCenterLocation = GetOwner()->GetActorLocation();
    }

    // Set up initial volcanic state
    SetVolcanicActivity(VolcanicActivity);

    UE_LOG(LogTemp, Warning, TEXT("VolcanicEnvironmentSystem: Initialized at location %s with radius %f"), 
           *VolcanicCenterLocation.ToString(), VolcanicZoneRadius);
}

void UEnvArt_VolcanicEnvironmentSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        // Update volcanic effects based on current activity
        switch (VolcanicActivity)
        {
        case EEnvArt_VolcanicActivity::Dormant:
            HandleDormantState();
            break;
        case EEnvArt_VolcanicActivity::Active:
            HandleActiveState();
            break;
        case EEnvArt_VolcanicActivity::Erupting:
            HandleEruptingState();
            break;
        case EEnvArt_VolcanicActivity::PostEruption:
            HandlePostEruptionState();
            break;
        }

        UpdateMaterialParameters();
        LastUpdateTime = CurrentTime;
    }
}

void UEnvArt_VolcanicEnvironmentSystem::SetVolcanicActivity(EEnvArt_VolcanicActivity NewActivity)
{
    if (VolcanicActivity != NewActivity)
    {
        VolcanicActivity = NewActivity;
        
        // Update effects based on new activity
        UpdateAshEffects();
        UpdateLavaEffects();
        UpdateVolcanicAudio();
        
        UE_LOG(LogTemp, Warning, TEXT("VolcanicEnvironmentSystem: Activity changed to %d"), (int32)NewActivity);
    }
}

void UEnvArt_VolcanicEnvironmentSystem::SpawnVolcanicProps()
{
    if (!GetWorld())
    {
        return;
    }

    // Clear existing props if we're at the limit
    if (CurrentVolcanicPropCount >= MaxVolcanicProps)
    {
        RemoveOldestProps(MaxVolcanicProps / 4); // Remove 25% of props
    }

    // Spawn different types of volcanic props
    SpawnLavaRocks();
    SpawnVolcanicBoulders();
    SpawnCooledLavaFlows();
    SpawnVolcanicVents();
    SpawnSulfurDeposits();

    UE_LOG(LogTemp, Warning, TEXT("VolcanicEnvironmentSystem: Spawned volcanic props. Total count: %d"), CurrentVolcanicPropCount);
}

void UEnvArt_VolcanicEnvironmentSystem::SpawnLavaRocks()
{
    if (VolcanicProps.LavaRocks.Num() == 0)
    {
        return;
    }

    int32 NumToSpawn = FMath::RandRange(15, 30);
    for (int32 i = 0; i < NumToSpawn && CurrentVolcanicPropCount < MaxVolcanicProps; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInZone();
        if (IsValidSpawnLocation(SpawnLocation))
        {
            UStaticMesh* SelectedMesh = VolcanicProps.LavaRocks[FMath::RandRange(0, VolcanicProps.LavaRocks.Num() - 1)];
            if (SelectedMesh)
            {
                AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>();
                if (PropActor)
                {
                    PropActor->GetStaticMeshComponent()->SetStaticMesh(SelectedMesh);
                    PropActor->SetActorLocation(SpawnLocation);
                    PropActor->SetActorRotation(FRotator(0, FMath::RandRange(0, 360), 0));
                    PropActor->SetActorScale3D(FVector(FMath::RandRange(0.8f, 1.5f)));
                    PropActor->Tags.Add(FName("VolcanicProp"));
                    PropActor->Tags.Add(FName("LavaRock"));

                    SpawnedVolcanicProps.Add(PropActor);
                    CurrentVolcanicPropCount++;
                }
            }
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::SpawnVolcanicBoulders()
{
    if (VolcanicProps.VolcanicBoulders.Num() == 0)
    {
        return;
    }

    int32 NumToSpawn = FMath::RandRange(8, 15);
    for (int32 i = 0; i < NumToSpawn && CurrentVolcanicPropCount < MaxVolcanicProps; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInZone();
        if (IsValidSpawnLocation(SpawnLocation))
        {
            UStaticMesh* SelectedMesh = VolcanicProps.VolcanicBoulders[FMath::RandRange(0, VolcanicProps.VolcanicBoulders.Num() - 1)];
            if (SelectedMesh)
            {
                AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>();
                if (PropActor)
                {
                    PropActor->GetStaticMeshComponent()->SetStaticMesh(SelectedMesh);
                    PropActor->SetActorLocation(SpawnLocation);
                    PropActor->SetActorRotation(FRotator(0, FMath::RandRange(0, 360), 0));
                    PropActor->SetActorScale3D(FVector(FMath::RandRange(1.2f, 2.5f)));
                    PropActor->Tags.Add(FName("VolcanicProp"));
                    PropActor->Tags.Add(FName("VolcanicBoulder"));

                    SpawnedVolcanicProps.Add(PropActor);
                    CurrentVolcanicPropCount++;
                }
            }
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::SpawnCooledLavaFlows()
{
    if (VolcanicProps.CooledLavaFlows.Num() == 0)
    {
        return;
    }

    int32 NumToSpawn = FMath::RandRange(5, 12);
    for (int32 i = 0; i < NumToSpawn && CurrentVolcanicPropCount < MaxVolcanicProps; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInZone();
        if (IsValidSpawnLocation(SpawnLocation))
        {
            UStaticMesh* SelectedMesh = VolcanicProps.CooledLavaFlows[FMath::RandRange(0, VolcanicProps.CooledLavaFlows.Num() - 1)];
            if (SelectedMesh)
            {
                AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>();
                if (PropActor)
                {
                    PropActor->GetStaticMeshComponent()->SetStaticMesh(SelectedMesh);
                    PropActor->SetActorLocation(SpawnLocation);
                    PropActor->SetActorRotation(FRotator(0, FMath::RandRange(0, 360), 0));
                    PropActor->SetActorScale3D(FVector(FMath::RandRange(1.0f, 1.8f)));
                    PropActor->Tags.Add(FName("VolcanicProp"));
                    PropActor->Tags.Add(FName("CooledLava"));

                    SpawnedVolcanicProps.Add(PropActor);
                    CurrentVolcanicPropCount++;
                }
            }
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::SpawnVolcanicVents()
{
    if (!VolcanicProps.VolcanicVent)
    {
        return;
    }

    int32 NumToSpawn = FMath::RandRange(3, 8);
    for (int32 i = 0; i < NumToSpawn && CurrentVolcanicPropCount < MaxVolcanicProps; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInZone();
        if (IsValidSpawnLocation(SpawnLocation))
        {
            AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>();
            if (PropActor)
            {
                PropActor->GetStaticMeshComponent()->SetStaticMesh(VolcanicProps.VolcanicVent);
                PropActor->SetActorLocation(SpawnLocation);
                PropActor->SetActorRotation(FRotator(0, FMath::RandRange(0, 360), 0));
                PropActor->Tags.Add(FName("VolcanicProp"));
                PropActor->Tags.Add(FName("VolcanicVent"));

                // Add steam particle effect if available
                if (SteamVentParticleSystem)
                {
                    UParticleSystemComponent* SteamComponent = UGameplayStatics::SpawnEmitterAttached(
                        SteamVentParticleSystem, PropActor->GetRootComponent(), NAME_None,
                        FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true
                    );
                }

                SpawnedVolcanicProps.Add(PropActor);
                CurrentVolcanicPropCount++;
            }
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::SpawnSulfurDeposits()
{
    if (!VolcanicProps.SulfurDeposits)
    {
        return;
    }

    int32 NumToSpawn = FMath::RandRange(10, 20);
    for (int32 i = 0; i < NumToSpawn && CurrentVolcanicPropCount < MaxVolcanicProps; ++i)
    {
        FVector SpawnLocation = GetRandomLocationInZone();
        if (IsValidSpawnLocation(SpawnLocation))
        {
            AStaticMeshActor* PropActor = GetWorld()->SpawnActor<AStaticMeshActor>();
            if (PropActor)
            {
                PropActor->GetStaticMeshComponent()->SetStaticMesh(VolcanicProps.SulfurDeposits);
                PropActor->SetActorLocation(SpawnLocation);
                PropActor->SetActorRotation(FRotator(0, FMath::RandRange(0, 360), 0));
                PropActor->SetActorScale3D(FVector(FMath::RandRange(0.5f, 1.2f)));
                PropActor->Tags.Add(FName("VolcanicProp"));
                PropActor->Tags.Add(FName("SulfurDeposit"));

                SpawnedVolcanicProps.Add(PropActor);
                CurrentVolcanicPropCount++;
            }
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::UpdateVolcanicAtmosphere()
{
    UpdateAshEffects();
    UpdateLavaEffects();
    UpdateMaterialParameters();
}

void UEnvArt_VolcanicEnvironmentSystem::UpdateAshEffects()
{
    if (!GetWorld())
    {
        return;
    }

    // Remove existing ash effect
    if (CurrentAshEffect && IsValid(CurrentAshEffect))
    {
        CurrentAshEffect->DestroyComponent();
        CurrentAshEffect = nullptr;
    }

    // Spawn new ash effect based on volcanic activity
    if (AshParticleSystem && (VolcanicActivity == EEnvArt_VolcanicActivity::Active || VolcanicActivity == EEnvArt_VolcanicActivity::Erupting))
    {
        CurrentAshEffect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), AshParticleSystem, VolcanicCenterLocation,
            FRotator::ZeroRotator, FVector(VolcanicAtmosphere.AshDensity), true
        );
    }
}

void UEnvArt_VolcanicEnvironmentSystem::UpdateLavaEffects()
{
    if (!GetWorld())
    {
        return;
    }

    // Remove existing lava effect
    if (CurrentLavaEffect && IsValid(CurrentLavaEffect))
    {
        CurrentLavaEffect->DestroyComponent();
        CurrentLavaEffect = nullptr;
    }

    // Spawn new lava effect for active/erupting states
    if (LavaGlowParticleSystem && (VolcanicActivity == EEnvArt_VolcanicActivity::Active || VolcanicActivity == EEnvArt_VolcanicActivity::Erupting))
    {
        CurrentLavaEffect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), LavaGlowParticleSystem, VolcanicCenterLocation,
            FRotator::ZeroRotator, FVector(VolcanicAtmosphere.LavaGlowIntensity), true
        );
    }
}

void UEnvArt_VolcanicEnvironmentSystem::UpdateVolcanicAudio()
{
    if (!GetWorld())
    {
        return;
    }

    // Stop existing audio
    if (CurrentVolcanicAudio && IsValid(CurrentVolcanicAudio))
    {
        CurrentVolcanicAudio->Stop();
        CurrentVolcanicAudio = nullptr;
    }

    // Play appropriate sound based on volcanic activity
    USoundCue* SoundToPlay = nullptr;
    switch (VolcanicActivity)
    {
    case EEnvArt_VolcanicActivity::Active:
        SoundToPlay = LavaBubbleSound;
        break;
    case EEnvArt_VolcanicActivity::Erupting:
        SoundToPlay = VolcanicRumbleSound;
        break;
    default:
        break;
    }

    if (SoundToPlay)
    {
        CurrentVolcanicAudio = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), SoundToPlay, VolcanicCenterLocation, 1.0f, 1.0f, 0.0f, nullptr, nullptr, true
        );
    }
}

void UEnvArt_VolcanicEnvironmentSystem::UpdateMaterialParameters()
{
    if (VolcanicMaterialParams)
    {
        UMaterialParameterCollectionInstance* ParamInstance = GetWorld()->GetParameterCollectionInstance(VolcanicMaterialParams);
        if (ParamInstance)
        {
            ParamInstance->SetVectorParameterValue(FName("VolcanicAshColor"), VolcanicAtmosphere.AshFogColor);
            ParamInstance->SetScalarParameterValue(FName("VolcanicAshDensity"), VolcanicAtmosphere.AshDensity);
            ParamInstance->SetVectorParameterValue(FName("VolcanicLavaGlow"), VolcanicAtmosphere.LavaGlowColor);
            ParamInstance->SetScalarParameterValue(FName("VolcanicLavaIntensity"), VolcanicAtmosphere.LavaGlowIntensity);
            ParamInstance->SetScalarParameterValue(FName("VolcanicActivity"), (float)VolcanicActivity);
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::ClearVolcanicProps()
{
    for (AActor* PropActor : SpawnedVolcanicProps)
    {
        if (IsValid(PropActor))
        {
            PropActor->Destroy();
        }
    }
    
    SpawnedVolcanicProps.Empty();
    CurrentVolcanicPropCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicEnvironmentSystem: Cleared all volcanic props"));
}

bool UEnvArt_VolcanicEnvironmentSystem::IsLocationInVolcanicZone(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, VolcanicCenterLocation);
    return Distance <= VolcanicZoneRadius;
}

float UEnvArt_VolcanicEnvironmentSystem::GetVolcanicIntensityAtLocation(const FVector& Location) const
{
    float Distance = FVector::Dist(Location, VolcanicCenterLocation);
    if (Distance >= VolcanicZoneRadius)
    {
        return 0.0f;
    }
    
    // Intensity decreases with distance from center
    float NormalizedDistance = Distance / VolcanicZoneRadius;
    float Intensity = 1.0f - NormalizedDistance;
    
    // Modify intensity based on volcanic activity
    switch (VolcanicActivity)
    {
    case EEnvArt_VolcanicActivity::Dormant:
        Intensity *= 0.2f;
        break;
    case EEnvArt_VolcanicActivity::Active:
        Intensity *= 0.7f;
        break;
    case EEnvArt_VolcanicActivity::Erupting:
        Intensity *= 1.5f;
        break;
    case EEnvArt_VolcanicActivity::PostEruption:
        Intensity *= 0.5f;
        break;
    }
    
    return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

bool UEnvArt_VolcanicEnvironmentSystem::IsValidSpawnLocation(const FVector& Location) const
{
    if (!GetWorld())
    {
        return false;
    }

    // Check if location is within volcanic zone
    if (!IsLocationInVolcanicZone(Location))
    {
        return false;
    }

    // Perform line trace to check if location is on ground
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 1000);
    FVector EndLocation = Location - FVector(0, 0, 1000);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, StartLocation, EndLocation, ECC_WorldStatic
    );

    return bHit && HitResult.bBlockingHit;
}

FVector UEnvArt_VolcanicEnvironmentSystem::GetRandomLocationInZone() const
{
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomRadius = FMath::RandRange(0.0f, VolcanicZoneRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomRadius,
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomRadius,
        0
    );
    
    return VolcanicCenterLocation + RandomOffset;
}

void UEnvArt_VolcanicEnvironmentSystem::RemoveOldestProps(int32 NumToRemove)
{
    int32 ActualRemoveCount = FMath::Min(NumToRemove, SpawnedVolcanicProps.Num());
    
    for (int32 i = 0; i < ActualRemoveCount; ++i)
    {
        if (SpawnedVolcanicProps.Num() > 0)
        {
            AActor* ActorToRemove = SpawnedVolcanicProps[0];
            if (IsValid(ActorToRemove))
            {
                ActorToRemove->Destroy();
            }
            SpawnedVolcanicProps.RemoveAt(0);
            CurrentVolcanicPropCount--;
        }
    }
}

void UEnvArt_VolcanicEnvironmentSystem::HandleDormantState()
{
    // Minimal effects for dormant volcano
    // Occasional steam vents, no lava glow
}

void UEnvArt_VolcanicEnvironmentSystem::HandleActiveState()
{
    // Moderate volcanic activity
    // Steam, some lava glow, occasional rumbles
}

void UEnvArt_VolcanicEnvironmentSystem::HandleEruptingState()
{
    // Maximum volcanic activity
    // Heavy ash, bright lava glow, constant rumbling
}

void UEnvArt_VolcanicEnvironmentSystem::HandlePostEruptionState()
{
    // Cooling down phase
    // Reduced effects, settling ash
}