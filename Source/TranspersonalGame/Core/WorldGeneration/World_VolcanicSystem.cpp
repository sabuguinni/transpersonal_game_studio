#include "World_VolcanicSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMeshActor.h"
#include "DrawDebugHelpers.h"

AWorld_VolcanicSystem::AWorld_VolcanicSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    InitializeComponents();
    
    // Initialize volcanic state
    bIsErupting = false;
    bInCooldown = false;
    CurrentEruptionTime = 0.0f;
    LastEruptionTime = 0.0f;
    EruptionTimer = 0.0f;
    CooldownTimer = 0.0f;
    
    // Set default volcanic config
    VolcanicConfig = FWorld_VolcanicConfig();
}

void AWorld_VolcanicSystem::InitializeComponents()
{
    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Create volcano base mesh
    VolcanoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VolcanoMesh"));
    VolcanoMesh->SetupAttachment(RootComponent);
    VolcanoMesh->SetRelativeScale3D(FVector(5.0f, 5.0f, 2.0f));
    
    // Create crater mesh
    CraterMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CraterMesh"));
    CraterMesh->SetupAttachment(VolcanoMesh);
    CraterMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 200.0f));
    CraterMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 0.3f));
    
    // Create particle effects
    LavaParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LavaParticles"));
    LavaParticles->SetupAttachment(CraterMesh);
    LavaParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));
    LavaParticles->SetAutoActivate(false);
    
    SmokeParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SmokeParticles"));
    SmokeParticles->SetupAttachment(CraterMesh);
    SmokeParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
    SmokeParticles->SetAutoActivate(true);
    
    AshCloudParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AshCloudParticles"));
    AshCloudParticles->SetupAttachment(CraterMesh);
    AshCloudParticles->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f));
    AshCloudParticles->SetAutoActivate(false);
    
    // Create audio component
    VolcanicAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VolcanicAudio"));
    VolcanicAudio->SetupAttachment(CraterMesh);
    VolcanicAudio->SetAutoActivate(false);
}

void AWorld_VolcanicSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply materials if available
    if (VolcanicRockMaterial && VolcanoMesh)
    {
        VolcanoMesh->SetMaterial(0, VolcanicRockMaterial);
    }
    
    if (LavaMaterial && CraterMesh)
    {
        CraterMesh->SetMaterial(0, LavaMaterial);
    }
    
    // Set up particle effects
    if (LavaFlowEffect && LavaParticles)
    {
        LavaParticles->SetAsset(LavaFlowEffect);
    }
    
    if (SmokeEffect && SmokeParticles)
    {
        SmokeParticles->SetAsset(SmokeEffect);
    }
    
    if (AshCloudEffect && AshCloudParticles)
    {
        AshCloudParticles->SetAsset(AshCloudEffect);
    }
    
    // Set up audio
    if (VolcanicRumbleSound && VolcanicAudio)
    {
        VolcanicAudio->SetSound(VolcanicRumbleSound);
    }
    
    // Create initial volcanic terrain
    CreateVolcanicTerrain();
    
    // Set initial activity state
    SetVolcanicActivity(VolcanicConfig.ActivityLevel);
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicSystem: Initialized at location %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateEruptionState(DeltaTime);
    UpdateParticleEffects();
    UpdateAudioEffects();
    HandleEruptionCycle();
}

void AWorld_VolcanicSystem::UpdateEruptionState(float DeltaTime)
{
    if (bIsErupting)
    {
        EruptionTimer += DeltaTime;
        CurrentEruptionTime = EruptionTimer;
        
        // Check if eruption should end
        if (EruptionTimer >= VolcanicConfig.EruptionDuration)
        {
            StopEruption();
        }
    }
    else if (bInCooldown)
    {
        CooldownTimer += DeltaTime;
        
        // Check if cooldown is complete
        if (CooldownTimer >= VolcanicConfig.CooldownTime)
        {
            bInCooldown = false;
            CooldownTimer = 0.0f;
            
            // Potentially start new eruption if active
            if (VolcanicConfig.ActivityLevel == EWorld_VolcanicActivity::Active)
            {
                // 30% chance to start new eruption after cooldown
                if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
                {
                    StartEruption();
                }
            }
        }
    }
}

void AWorld_VolcanicSystem::UpdateParticleEffects()
{
    if (!LavaParticles || !SmokeParticles || !AshCloudParticles)
        return;
    
    // Update lava particles based on eruption state
    if (bIsErupting)
    {
        if (!LavaParticles->IsActive())
        {
            LavaParticles->Activate();
        }
        
        // Increase intensity over time
        float EruptionProgress = GetEruptionProgress();
        float LavaIntensity = FMath::Lerp(0.5f, 2.0f, EruptionProgress);
        
        // Scale particle emission based on eruption progress
        LavaParticles->SetFloatParameter(TEXT("EmissionRate"), LavaIntensity * 100.0f);
    }
    else
    {
        if (LavaParticles->IsActive())
        {
            LavaParticles->Deactivate();
        }
    }
    
    // Update smoke based on activity level
    if (VolcanicConfig.ActivityLevel != EWorld_VolcanicActivity::Dormant)
    {
        if (!SmokeParticles->IsActive())
        {
            SmokeParticles->Activate();
        }
        
        float SmokeIntensity = (VolcanicConfig.ActivityLevel == EWorld_VolcanicActivity::Erupting) ? 2.0f : 0.5f;
        SmokeParticles->SetFloatParameter(TEXT("EmissionRate"), SmokeIntensity * 50.0f);
    }
    else
    {
        if (SmokeParticles->IsActive())
        {
            SmokeParticles->Deactivate();
        }
    }
    
    // Update ash cloud during eruptions
    if (bIsErupting && GetEruptionProgress() > 0.3f)
    {
        if (!AshCloudParticles->IsActive())
        {
            AshCloudParticles->Activate();
        }
    }
    else
    {
        if (AshCloudParticles->IsActive())
        {
            AshCloudParticles->Deactivate();
        }
    }
}

void AWorld_VolcanicSystem::UpdateAudioEffects()
{
    if (!VolcanicAudio)
        return;
    
    if (bIsErupting)
    {
        if (!VolcanicAudio->IsPlaying())
        {
            if (EruptionSound)
            {
                VolcanicAudio->SetSound(EruptionSound);
            }
            VolcanicAudio->Play();
        }
        
        // Adjust volume based on eruption progress
        float Volume = FMath::Lerp(0.3f, 1.0f, GetEruptionProgress());
        VolcanicAudio->SetVolumeMultiplier(Volume);
    }
    else if (VolcanicConfig.ActivityLevel == EWorld_VolcanicActivity::Active)
    {
        if (!VolcanicAudio->IsPlaying())
        {
            if (VolcanicRumbleSound)
            {
                VolcanicAudio->SetSound(VolcanicRumbleSound);
            }
            VolcanicAudio->Play();
        }
        VolcanicAudio->SetVolumeMultiplier(0.2f);
    }
    else
    {
        if (VolcanicAudio->IsPlaying())
        {
            VolcanicAudio->Stop();
        }
    }
}

void AWorld_VolcanicSystem::HandleEruptionCycle()
{
    // Auto-eruption logic for active volcanoes
    if (VolcanicConfig.ActivityLevel == EWorld_VolcanicActivity::Active && !bIsErupting && !bInCooldown)
    {
        // Check if enough time has passed since last eruption
        float TimeSinceLastEruption = GetWorld()->GetTimeSeconds() - LastEruptionTime;
        
        if (TimeSinceLastEruption > VolcanicConfig.CooldownTime * 2.0f)
        {
            // Random chance for eruption (5% per tick when conditions are met)
            if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
            {
                StartEruption();
            }
        }
    }
}

void AWorld_VolcanicSystem::StartEruption()
{
    if (bIsErupting)
        return;
    
    bIsErupting = true;
    bInCooldown = false;
    EruptionTimer = 0.0f;
    LastEruptionTime = GetWorld()->GetTimeSeconds();
    
    // Update activity level
    VolcanicConfig.ActivityLevel = EWorld_VolcanicActivity::Erupting;
    
    // Create lava flows
    CreateLavaFlows();
    
    // Spawn volcanic rocks
    SpawnVolcanicRocks(25);
    
    // Update effects
    UpdateVolcanicEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicSystem: Eruption started at %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicSystem::StopEruption()
{
    if (!bIsErupting)
        return;
    
    bIsErupting = false;
    bInCooldown = true;
    CooldownTimer = 0.0f;
    CurrentEruptionTime = 0.0f;
    
    // Return to active state
    VolcanicConfig.ActivityLevel = EWorld_VolcanicActivity::Active;
    
    // Update effects
    UpdateVolcanicEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicSystem: Eruption stopped, entering cooldown"));
}

void AWorld_VolcanicSystem::SetVolcanicActivity(EWorld_VolcanicActivity NewActivity)
{
    VolcanicConfig.ActivityLevel = NewActivity;
    
    // Stop eruption if setting to dormant
    if (NewActivity == EWorld_VolcanicActivity::Dormant && bIsErupting)
    {
        StopEruption();
    }
    
    UpdateVolcanicEffects();
}

EWorld_VolcanicActivity AWorld_VolcanicSystem::GetVolcanicActivity() const
{
    return VolcanicConfig.ActivityLevel;
}

float AWorld_VolcanicSystem::GetEruptionProgress() const
{
    if (!bIsErupting || VolcanicConfig.EruptionDuration <= 0.0f)
        return 0.0f;
    
    return FMath::Clamp(EruptionTimer / VolcanicConfig.EruptionDuration, 0.0f, 1.0f);
}

void AWorld_VolcanicSystem::UpdateVolcanicEffects()
{
    UpdateParticleEffects();
    UpdateAudioEffects();
}

void AWorld_VolcanicSystem::CreateLavaFlows()
{
    if (!GetWorld())
        return;
    
    // Clear existing lava flows
    for (AActor* LavaActor : LavaFlowActors)
    {
        if (IsValid(LavaActor))
        {
            LavaActor->Destroy();
        }
    }
    LavaFlowActors.Empty();
    
    // Create new lava flows radiating from crater
    int32 NumFlows = FMath::RandRange(3, 6);
    FVector CraterLoc = GetCraterLocation();
    
    for (int32 i = 0; i < NumFlows; i++)
    {
        float Angle = (360.0f / NumFlows) * i + FMath::RandRange(-30.0f, 30.0f);
        FVector FlowDirection = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)),
            FMath::Sin(FMath::DegreesToRadians(Angle)),
            -0.3f // Flow downward
        ).GetSafeNormal();
        
        // Create lava flow segments
        int32 NumSegments = FMath::RandRange(5, 10);
        FVector CurrentLocation = CraterLoc;
        
        for (int32 j = 0; j < NumSegments; j++)
        {
            AStaticMeshActor* LavaSegment = GetWorld()->SpawnActor<AStaticMeshActor>();
            if (LavaSegment)
            {
                // Position segment
                CurrentLocation += FlowDirection * FMath::RandRange(80.0f, 120.0f);
                LavaSegment->SetActorLocation(CurrentLocation);
                LavaSegment->SetActorScale3D(FVector(FMath::RandRange(0.8f, 1.2f)));
                
                // Apply lava material if available
                if (LavaMaterial && LavaSegment->GetStaticMeshComponent())
                {
                    LavaSegment->GetStaticMeshComponent()->SetMaterial(0, LavaMaterial);
                }
                
                LavaFlowActors.Add(LavaSegment);
                
                // Slight direction variation for natural flow
                FlowDirection += FVector(
                    FMath::RandRange(-0.2f, 0.2f),
                    FMath::RandRange(-0.2f, 0.2f),
                    FMath::RandRange(-0.1f, 0.1f)
                );
                FlowDirection.Normalize();
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VolcanicSystem: Created %d lava flow actors"), LavaFlowActors.Num());
}

void AWorld_VolcanicSystem::SpawnVolcanicRocks(int32 NumRocks)
{
    if (!GetWorld())
        return;
    
    // Clear existing rocks
    for (AActor* RockActor : VolcanicRockActors)
    {
        if (IsValid(RockActor))
        {
            RockActor->Destroy();
        }
    }
    VolcanicRockActors.Empty();
    
    FVector CraterLoc = GetCraterLocation();
    
    for (int32 i = 0; i < NumRocks; i++)
    {
        AStaticMeshActor* VolcanicRock = GetWorld()->SpawnActor<AStaticMeshActor>();
        if (VolcanicRock)
        {
            // Random position around volcano
            float Distance = FMath::RandRange(200.0f, VolcanicConfig.LavaFlowRange);
            float Angle = FMath::RandRange(0.0f, 360.0f);
            
            FVector RockLocation = CraterLoc + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
                FMath::RandRange(-50.0f, 100.0f)
            );
            
            VolcanicRock->SetActorLocation(RockLocation);
            VolcanicRock->SetActorRotation(FRotator(
                FMath::RandRange(-45.0f, 45.0f),
                FMath::RandRange(0.0f, 360.0f),
                FMath::RandRange(-45.0f, 45.0f)
            ));
            VolcanicRock->SetActorScale3D(FVector(FMath::RandRange(0.5f, 2.0f)));
            
            // Apply volcanic rock material
            if (VolcanicRockMaterial && VolcanicRock->GetStaticMeshComponent())
            {
                VolcanicRock->GetStaticMeshComponent()->SetMaterial(0, VolcanicRockMaterial);
            }
            
            VolcanicRockActors.Add(VolcanicRock);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VolcanicSystem: Spawned %d volcanic rocks"), VolcanicRockActors.Num());
}

FVector AWorld_VolcanicSystem::GetCraterLocation() const
{
    if (CraterMesh)
    {
        return CraterMesh->GetComponentLocation();
    }
    return GetActorLocation() + FVector(0.0f, 0.0f, VolcanicConfig.VolcanoHeight);
}

float AWorld_VolcanicSystem::GetDistanceFromCrater(FVector Location) const
{
    return FVector::Dist(Location, GetCraterLocation());
}

bool AWorld_VolcanicSystem::IsInAshCloudRange(FVector Location) const
{
    return GetDistanceFromCrater(Location) <= VolcanicConfig.AshCloudRadius;
}

bool AWorld_VolcanicSystem::IsInLavaFlowRange(FVector Location) const
{
    return GetDistanceFromCrater(Location) <= VolcanicConfig.LavaFlowRange;
}

void AWorld_VolcanicSystem::CreateVolcanicTerrain()
{
    // This would integrate with landscape/terrain system
    // For now, just log the intention
    UE_LOG(LogTemp, Log, TEXT("VolcanicSystem: Creating volcanic terrain at %s"), *GetActorLocation().ToString());
}

void AWorld_VolcanicSystem::SpawnLavaPool()
{
    if (!GetWorld())
        return;
    
    AStaticMeshActor* LavaPool = GetWorld()->SpawnActor<AStaticMeshActor>();
    if (LavaPool)
    {
        LavaPool->SetActorLocation(GetCraterLocation() + FVector(0.0f, 0.0f, -20.0f));
        LavaPool->SetActorScale3D(FVector(2.0f, 2.0f, 0.1f));
        
        if (LavaMaterial && LavaPool->GetStaticMeshComponent())
        {
            LavaPool->GetStaticMeshComponent()->SetMaterial(0, LavaMaterial);
        }
    }
}

void AWorld_VolcanicSystem::UpdateLavaTemperature()
{
    // Temperature affects nearby environment
    // This would integrate with weather/biome systems
    float Temperature = bIsErupting ? 1200.0f : (VolcanicConfig.ActivityLevel == EWorld_VolcanicActivity::Active ? 400.0f : 100.0f);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("VolcanicSystem: Lava temperature: %f"), Temperature);
}