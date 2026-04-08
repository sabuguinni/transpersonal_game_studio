// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ConsciousnessZone.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"
#include "TranspersonalGame/Core/ConsciousnessSystem.h"

AConsciousnessZone::AConsciousnessZone()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create zone collision
    ZoneCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneCollision"));
    ZoneCollision->SetupAttachment(RootComponent);
    ZoneCollision->SetBoxExtent(FVector(500.0f, 500.0f, 200.0f));
    ZoneCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ZoneCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    ZoneCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    // Create visualization mesh
    ZoneVisualization = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneVisualization"));
    ZoneVisualization->SetupAttachment(RootComponent);
    ZoneVisualization->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Create audio component
    ZoneAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ZoneAudio"));
    ZoneAudio->SetupAttachment(RootComponent);
    ZoneAudio->bAutoActivate = false;

    // Create particle system
    ZoneParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ZoneParticles"));
    ZoneParticles->SetupAttachment(RootComponent);
    ZoneParticles->bAutoActivate = false;

    // Initialize variables
    bIsZoneActive = false;
    ZoneIntensity = 0.0f;
    LastConsciousnessLevel = 0.0f;
}

void AConsciousnessZone::BeginPlay()
{
    Super::BeginPlay();

    // Bind overlap events
    ZoneCollision->OnComponentBeginOverlap.AddDynamic(this, &AConsciousnessZone::OnZoneBeginOverlap);
    ZoneCollision->OnComponentEndOverlap.AddDynamic(this, &AConsciousnessZone::OnZoneEndOverlap);

    // Activate zone by default
    ActivateZone();
}

void AConsciousnessZone::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsZoneActive)
        return;

    // Update zone for each player inside
    for (ATranspersonalCharacter* Player : PlayersInZone)
    {
        if (IsValid(Player))
        {
            UpdateZoneForPlayer(Player, DeltaTime);
        }
    }

    // Clean up invalid player references
    PlayersInZone.RemoveAll([](ATranspersonalCharacter* Player) {
        return !IsValid(Player);
    });

    // Update zone effects
    float AverageConsciousness = 0.0f;
    if (PlayersInZone.Num() > 0)
    {
        for (ATranspersonalCharacter* Player : PlayersInZone)
        {
            if (IsValid(Player) && Player->GetConsciousnessSystem())
            {
                AverageConsciousness += Player->GetConsciousnessSystem()->GetCurrentLevel();
            }
        }
        AverageConsciousness /= PlayersInZone.Num();
    }

    // Update zone intensity based on consciousness
    float TargetIntensity = AverageConsciousness * ZoneConfig.InfluenceStrength;
    ZoneIntensity = FMath::FInterpTo(ZoneIntensity, TargetIntensity, DeltaTime, 2.0f);

    // Call Blueprint event for visual updates
    UpdateZoneEffects(AverageConsciousness, DeltaTime);

    // Check for consciousness threshold events
    if (FMath::Abs(AverageConsciousness - LastConsciousnessLevel) > 0.1f)
    {
        if (AverageConsciousness >= ZoneConfig.RequiredConsciousnessLevel && 
            LastConsciousnessLevel < ZoneConfig.RequiredConsciousnessLevel)
        {
            for (ATranspersonalCharacter* Player : PlayersInZone)
            {
                if (IsValid(Player))
                {
                    OnConsciousnessThresholdReached.Broadcast(Player, AverageConsciousness, ZoneConfig.ZoneType);
                }
            }
        }
        LastConsciousnessLevel = AverageConsciousness;
    }
}

void AConsciousnessZone::ActivateZone()
{
    bIsZoneActive = true;
    
    if (ZoneConfig.bEnableAudioEffects && ZoneAudio)
    {
        ZoneAudio->Activate();
    }
    
    if (ZoneConfig.bEnableParticleEffects && ZoneParticles)
    {
        ZoneParticles->Activate();
    }

    OnZoneActivated();
}

void AConsciousnessZone::DeactivateZone()
{
    bIsZoneActive = false;
    
    if (ZoneAudio)
    {
        ZoneAudio->Deactivate();
    }
    
    if (ZoneParticles)
    {
        ZoneParticles->Deactivate();
    }

    OnZoneDeactivated();
}

bool AConsciousnessZone::IsPlayerInZone(ATranspersonalCharacter* Player) const
{
    return PlayersInZone.Contains(Player);
}

float AConsciousnessZone::GetConsciousnessInfluenceForPlayer(ATranspersonalCharacter* Player) const
{
    if (!IsValid(Player) || !IsPlayerInZone(Player))
        return 0.0f;

    // Calculate distance-based influence
    float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
    float MaxDistance = ZoneConfig.ConsciousnessInfluenceRadius;
    
    if (Distance >= MaxDistance)
        return 0.0f;

    float DistanceRatio = 1.0f - (Distance / MaxDistance);
    return DistanceRatio * ZoneConfig.InfluenceStrength * ZoneIntensity;
}

void AConsciousnessZone::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, 
                                           bool bFromSweep, const FHitResult& SweepResult)
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor);
    if (IsValid(Player) && !PlayersInZone.Contains(Player))
    {
        PlayersInZone.Add(Player);
        OnPlayerEnterZone.Broadcast(Player, ZoneConfig.ZoneType);

        UE_LOG(LogTemp, Log, TEXT("Player entered consciousness zone: %s"), 
               *UEnum::GetValueAsString(ZoneConfig.ZoneType));
    }
}

void AConsciousnessZone::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(OtherActor);
    if (IsValid(Player) && PlayersInZone.Contains(Player))
    {
        PlayersInZone.Remove(Player);
        OnPlayerExitZone.Broadcast(Player, ZoneConfig.ZoneType);

        UE_LOG(LogTemp, Log, TEXT("Player exited consciousness zone: %s"), 
               *UEnum::GetValueAsString(ZoneConfig.ZoneType));
    }
}

void AConsciousnessZone::UpdateZoneForPlayer(ATranspersonalCharacter* Player, float DeltaTime)
{
    if (!IsValid(Player))
        return;

    ApplyConsciousnessInfluence(Player, DeltaTime);
}

void AConsciousnessZone::ApplyConsciousnessInfluence(ATranspersonalCharacter* Player, float DeltaTime)
{
    if (!IsValid(Player) || !Player->GetConsciousnessSystem())
        return;

    float Influence = GetConsciousnessInfluenceForPlayer(Player);
    
    if (Influence > 0.0f)
    {
        // Apply different effects based on zone type
        switch (ZoneConfig.ZoneType)
        {
            case EConsciousnessZoneType::Meditation:
                // Gradually increase consciousness in meditation zones
                Player->GetConsciousnessSystem()->ModifyConsciousness(Influence * DeltaTime * 0.1f);
                break;
                
            case EConsciousnessZoneType::Transformation:
                // Accelerate consciousness changes
                Player->GetConsciousnessSystem()->ModifyConsciousness(Influence * DeltaTime * 0.2f);
                break;
                
            case EConsciousnessZoneType::Challenge:
                // Create consciousness fluctuations
                float Fluctuation = FMath::Sin(GetWorld()->GetTimeSeconds() * 2.0f) * Influence * DeltaTime * 0.05f;
                Player->GetConsciousnessSystem()->ModifyConsciousness(Fluctuation);
                break;
                
            case EConsciousnessZoneType::Sanctuary:
                // Stabilize consciousness at current level
                // No modification, just maintain current state
                break;
                
            case EConsciousnessZoneType::Vortex:
                // Rapid consciousness changes
                Player->GetConsciousnessSystem()->ModifyConsciousness(Influence * DeltaTime * 0.3f);
                break;
                
            case EConsciousnessZoneType::Portal:
                // Threshold-based consciousness jumps
                if (Player->GetConsciousnessSystem()->GetCurrentLevel() >= ZoneConfig.RequiredConsciousnessLevel)
                {
                    Player->GetConsciousnessSystem()->ModifyConsciousness(Influence * DeltaTime * 0.5f);
                }
                break;
        }
    }
}