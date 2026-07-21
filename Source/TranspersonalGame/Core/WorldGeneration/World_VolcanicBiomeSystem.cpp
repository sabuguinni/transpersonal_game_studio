#include "World_VolcanicBiomeSystem.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UWorld_VolcanicBiomeSystem::UWorld_VolcanicBiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms

    // Initialize default values
    BaseTemperature = 35.0f; // Celsius
    VolcanicActivityLevel = 0.5f;

    // Initialize arrays
    VolcanicFeatures.Empty();
    LavaFlows.Empty();
    SpawnedMeshComponents.Empty();
    SpawnedParticleComponents.Empty();
    SpawnedAudioComponents.Empty();

    // Asset pointers will be set via Blueprint or C++
    SteamVentMesh = nullptr;
    LavaPoolMesh = nullptr;
    VolcanicRockMesh = nullptr;
    FernMesh = nullptr;
    CycadMesh = nullptr;
    LavaMaterial = nullptr;
    VolcanicRockMaterial = nullptr;
    AshMaterial = nullptr;
    SteamParticles = nullptr;
    AshParticles = nullptr;
    LavaBubbleParticles = nullptr;
    HeatDistortionParticles = nullptr;
    VolcanicAmbientSound = nullptr;
    LavaBubbleSound = nullptr;
    SteamHissSound = nullptr;
    AmbientAudioComponent = nullptr;
}

void UWorld_VolcanicBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeVolcanicAssets();
    
    // Create ambient audio component
    if (VolcanicAmbientSound && !AmbientAudioComponent)
    {
        AmbientAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
            GetWorld(), 
            VolcanicAmbientSound, 
            GetOwner()->GetActorLocation(),
            FRotator::ZeroRotator,
            0.5f, // Volume
            1.0f, // Pitch
            0.0f, // Start time
            nullptr, // Attenuation
            nullptr, // Concurrency
            true // Auto destroy
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeSystem: BeginPlay completed"));
}

void UWorld_VolcanicBiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateLavaFlowAnimation(DeltaTime);
    UpdateParticleEffects(DeltaTime);
    UpdateTemperatureField();
}

void UWorld_VolcanicBiomeSystem::GenerateVolcanicFeatures(const FVector& CenterLocation, float BiomeRadius)
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("VolcanicBiomeSystem: No valid world for feature generation"));
        return;
    }

    // Clear existing features
    VolcanicFeatures.Empty();
    CleanupSpawnedComponents();

    // Generate steam vents (8-12 per biome)
    int32 SteamVentCount = FMath::RandRange(8, 12);
    for (int32 i = 0; i < SteamVentCount; i++)
    {
        FVector Location = FindSuitableVolcanicLocation(CenterLocation, BiomeRadius * 0.8f);
        if (Location != FVector::ZeroVector)
        {
            SpawnSteamVent(Location, FMath::RandRange(0.3f, 0.9f));
        }
    }

    // Generate lava pools (3-5 per biome)
    int32 LavaPoolCount = FMath::RandRange(3, 5);
    for (int32 i = 0; i < LavaPoolCount; i++)
    {
        FVector Location = FindSuitableVolcanicLocation(CenterLocation, BiomeRadius * 0.6f);
        if (Location != FVector::ZeroVector)
        {
            SpawnLavaPool(Location, FMath::RandRange(300.0f, 800.0f));
        }
    }

    // Generate lava flows (2-3 per biome)
    int32 LavaFlowCount = FMath::RandRange(2, 3);
    for (int32 i = 0; i < LavaFlowCount; i++)
    {
        FVector StartLocation = FindSuitableVolcanicLocation(CenterLocation, BiomeRadius * 0.4f);
        FVector EndLocation = FindSuitableVolcanicLocation(CenterLocation, BiomeRadius * 0.9f);
        if (StartLocation != FVector::ZeroVector && EndLocation != FVector::ZeroVector)
        {
            CreateLavaFlow(StartLocation, EndLocation);
        }
    }

    // Generate volcanic rocks (15-25 per biome)
    int32 RockCount = FMath::RandRange(15, 25);
    for (int32 i = 0; i < RockCount; i++)
    {
        FVector Location = FindSuitableVolcanicLocation(CenterLocation, BiomeRadius);
        if (Location != FVector::ZeroVector)
        {
            SpawnVolcanicRock(Location, FMath::RandRange(0.5f, 2.0f));
        }
    }

    // Generate heat-resistant vegetation
    int32 VegetationCount = FMath::RandRange(20, 40);
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector Location = FindSuitableVolcanicLocation(CenterLocation, BiomeRadius);
        if (Location != FVector::ZeroVector && GetTemperatureAtLocation(Location) < 60.0f)
        {
            SpawnVolcanicVegetation(Location);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeSystem: Generated %d features in biome"), VolcanicFeatures.Num());
}

void UWorld_VolcanicBiomeSystem::SpawnSteamVent(const FVector& Location, float Intensity)
{
    if (!GetWorld())
        return;

    // Create volcanic feature data
    FWorld_VolcanicFeature Feature;
    Feature.Location = Location;
    Feature.Intensity = Intensity;
    Feature.FeatureType = EWorld_VolcanicFeatureType::SteamVent;
    Feature.Radius = 100.0f;
    Feature.bIsActive = true;
    VolcanicFeatures.Add(Feature);

    // Spawn mesh if available
    if (SteamVentMesh)
    {
        AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location,
            FRotator::ZeroRotator
        );
        
        if (MeshActor)
        {
            MeshActor->GetStaticMeshComponent()->SetStaticMesh(SteamVentMesh);
            MeshActor->SetActorLabel(FString::Printf(TEXT("SteamVent_%d"), VolcanicFeatures.Num()));
            SpawnedMeshComponents.Add(MeshActor->GetStaticMeshComponent());
        }
    }

    // Spawn steam particles
    if (SteamParticles)
    {
        UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            SteamParticles,
            Location + FVector(0, 0, 50),
            FRotator::ZeroRotator,
            FVector(Intensity, Intensity, Intensity),
            true
        );
        
        if (ParticleComp)
        {
            SpawnedParticleComponents.Add(ParticleComp);
        }
    }

    // Play steam sound
    PlaySteamHissSound(Location);
}

void UWorld_VolcanicBiomeSystem::SpawnLavaPool(const FVector& Location, float Radius)
{
    if (!GetWorld())
        return;

    // Create volcanic feature data
    FWorld_VolcanicFeature Feature;
    Feature.Location = Location;
    Feature.Intensity = 1.0f;
    Feature.FeatureType = EWorld_VolcanicFeatureType::LavaPool;
    Feature.Radius = Radius;
    Feature.bIsActive = true;
    VolcanicFeatures.Add(Feature);

    // Spawn mesh if available
    if (LavaPoolMesh)
    {
        AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location,
            FRotator::ZeroRotator
        );
        
        if (MeshActor)
        {
            MeshActor->GetStaticMeshComponent()->SetStaticMesh(LavaPoolMesh);
            MeshActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(Radius / 500.0f));
            
            // Apply lava material
            if (LavaMaterial)
            {
                MeshActor->GetStaticMeshComponent()->SetMaterial(0, LavaMaterial);
            }
            
            MeshActor->SetActorLabel(FString::Printf(TEXT("LavaPool_%d"), VolcanicFeatures.Num()));
            SpawnedMeshComponents.Add(MeshActor->GetStaticMeshComponent());
        }
    }

    // Spawn lava bubble particles
    if (LavaBubbleParticles)
    {
        UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            LavaBubbleParticles,
            Location + FVector(0, 0, 10),
            FRotator::ZeroRotator,
            FVector(Radius / 500.0f),
            true
        );
        
        if (ParticleComp)
        {
            SpawnedParticleComponents.Add(ParticleComp);
        }
    }

    // Play lava bubble sound
    PlayLavaBubbleSound(Location);
}

void UWorld_VolcanicBiomeSystem::CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation)
{
    FWorld_LavaFlowData LavaFlow;
    LavaFlow.FlowPath.Add(StartLocation);
    
    // Create intermediate points for curved flow
    FVector Direction = (EndLocation - StartLocation).GetSafeNormal();
    float Distance = FVector::Dist(StartLocation, EndLocation);
    int32 SegmentCount = FMath::Max(3, FMath::FloorToInt(Distance / 200.0f));
    
    for (int32 i = 1; i < SegmentCount; i++)
    {
        float Alpha = float(i) / float(SegmentCount);
        FVector SegmentLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
        
        // Add some randomness to make flow look natural
        SegmentLocation += FVector(
            FMath::RandRange(-100.0f, 100.0f),
            FMath::RandRange(-100.0f, 100.0f),
            0
        );
        
        LavaFlow.FlowPath.Add(SegmentLocation);
    }
    
    LavaFlow.FlowPath.Add(EndLocation);
    LavaFlow.FlowSpeed = FMath::RandRange(50.0f, 150.0f);
    LavaFlow.Temperature = FMath::RandRange(1000.0f, 1300.0f);
    LavaFlow.Width = FMath::RandRange(100.0f, 300.0f);
    LavaFlow.bIsFlowing = true;
    
    LavaFlows.Add(LavaFlow);
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeSystem: Created lava flow with %d segments"), LavaFlow.FlowPath.Num());
}

void UWorld_VolcanicBiomeSystem::SpawnVolcanicRock(const FVector& Location, float Scale)
{
    if (!GetWorld() || !VolcanicRockMesh)
        return;

    AStaticMeshActor* RockActor = GetWorld()->SpawnActor<AStaticMeshActor>(
        AStaticMeshActor::StaticClass(),
        Location,
        FRotator(
            FMath::RandRange(-15.0f, 15.0f),
            FMath::RandRange(0.0f, 360.0f),
            FMath::RandRange(-10.0f, 10.0f)
        )
    );
    
    if (RockActor)
    {
        RockActor->GetStaticMeshComponent()->SetStaticMesh(VolcanicRockMesh);
        RockActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(Scale));
        
        if (VolcanicRockMaterial)
        {
            RockActor->GetStaticMeshComponent()->SetMaterial(0, VolcanicRockMaterial);
        }
        
        RockActor->SetActorLabel(FString::Printf(TEXT("VolcanicRock_%f"), Scale));
        SpawnedMeshComponents.Add(RockActor->GetStaticMeshComponent());
    }
}

void UWorld_VolcanicBiomeSystem::UpdateVolcanicActivity(float ActivityLevel)
{
    VolcanicActivityLevel = FMath::Clamp(ActivityLevel, 0.0f, 1.0f);
    
    // Update all volcanic features based on activity level
    for (FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        Feature.bIsActive = (VolcanicActivityLevel > 0.1f);
        
        if (Feature.FeatureType == EWorld_VolcanicFeatureType::SteamVent)
        {
            Feature.Intensity = VolcanicActivityLevel * 0.8f;
        }
    }
    
    // Update lava flows
    for (FWorld_LavaFlowData& Flow : LavaFlows)
    {
        Flow.bIsFlowing = (VolcanicActivityLevel > 0.2f);
        Flow.FlowSpeed = 100.0f * VolcanicActivityLevel;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeSystem: Activity level updated to %f"), VolcanicActivityLevel);
}

void UWorld_VolcanicBiomeSystem::SpawnAshParticles(const FVector& Location, float Intensity)
{
    if (!GetWorld() || !AshParticles)
        return;

    UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        AshParticles,
        Location + FVector(0, 0, 200),
        FRotator::ZeroRotator,
        FVector(Intensity),
        true
    );
    
    if (ParticleComp)
    {
        SpawnedParticleComponents.Add(ParticleComp);
    }
}

void UWorld_VolcanicBiomeSystem::UpdateHeatDistortion(float DistortionStrength)
{
    // Update heat distortion effects for all lava features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        if (Feature.FeatureType == EWorld_VolcanicFeatureType::LavaPool && HeatDistortionParticles)
        {
            UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                HeatDistortionParticles,
                Feature.Location + FVector(0, 0, 100),
                FRotator::ZeroRotator,
                FVector(DistortionStrength),
                false
            );
        }
    }
}

void UWorld_VolcanicBiomeSystem::SpawnVolcanicVegetation(const FVector& Location)
{
    if (!GetWorld())
        return;

    // Randomly choose between ferns and cycads
    bool bSpawnFern = FMath::RandBool();
    
    if (bSpawnFern && FernMesh)
    {
        AStaticMeshActor* FernActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location,
            FRotator(0, FMath::RandRange(0.0f, 360.0f), 0)
        );
        
        if (FernActor)
        {
            FernActor->GetStaticMeshComponent()->SetStaticMesh(FernMesh);
            FernActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(FMath::RandRange(0.8f, 1.2f)));
            FernActor->SetActorLabel(TEXT("VolcanicFern"));
            SpawnedMeshComponents.Add(FernActor->GetStaticMeshComponent());
        }
    }
    else if (CycadMesh)
    {
        AStaticMeshActor* CycadActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location,
            FRotator(0, FMath::RandRange(0.0f, 360.0f), 0)
        );
        
        if (CycadActor)
        {
            CycadActor->GetStaticMeshComponent()->SetStaticMesh(CycadMesh);
            CycadActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(FMath::RandRange(0.6f, 1.4f)));
            CycadActor->SetActorLabel(TEXT("VolcanicCycad"));
            SpawnedMeshComponents.Add(CycadActor->GetStaticMeshComponent());
        }
    }
}

void UWorld_VolcanicBiomeSystem::PlaceFerns(const FVector& CenterLocation, float Radius, int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        FVector Location = CenterLocation + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0
        );
        
        if (GetTemperatureAtLocation(Location) < 50.0f) // Ferns can't survive extreme heat
        {
            SpawnVolcanicVegetation(Location);
        }
    }
}

void UWorld_VolcanicBiomeSystem::PlaceCycads(const FVector& CenterLocation, float Radius, int32 Count)
{
    for (int32 i = 0; i < Count; i++)
    {
        FVector Location = CenterLocation + FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0
        );
        
        if (GetTemperatureAtLocation(Location) < 70.0f) // Cycads are more heat resistant
        {
            SpawnVolcanicVegetation(Location);
        }
    }
}

void UWorld_VolcanicBiomeSystem::UpdateVolcanicAmbientSounds()
{
    if (AmbientAudioComponent && VolcanicAmbientSound)
    {
        // Adjust volume based on volcanic activity
        float Volume = 0.3f + (VolcanicActivityLevel * 0.4f);
        AmbientAudioComponent->SetVolumeMultiplier(Volume);
    }
}

void UWorld_VolcanicBiomeSystem::PlayLavaBubbleSound(const FVector& Location)
{
    if (LavaBubbleSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            LavaBubbleSound,
            Location,
            FRotator::ZeroRotator,
            0.6f, // Volume
            FMath::RandRange(0.8f, 1.2f) // Random pitch
        );
    }
}

void UWorld_VolcanicBiomeSystem::PlaySteamHissSound(const FVector& Location)
{
    if (SteamHissSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            SteamHissSound,
            Location,
            FRotator::ZeroRotator,
            0.4f, // Volume
            FMath::RandRange(0.9f, 1.1f) // Random pitch
        );
    }
}

float UWorld_VolcanicBiomeSystem::GetTemperatureAtLocation(const FVector& Location) const
{
    float Temperature = BaseTemperature;
    
    // Add heat from volcanic features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        float HeatInfluence = 0.0f;
        
        switch (Feature.FeatureType)
        {
            case EWorld_VolcanicFeatureType::LavaPool:
                if (Distance < Feature.Radius * 2.0f)
                {
                    HeatInfluence = (1.0f - (Distance / (Feature.Radius * 2.0f))) * 60.0f * Feature.Intensity;
                }
                break;
                
            case EWorld_VolcanicFeatureType::SteamVent:
                if (Distance < 300.0f)
                {
                    HeatInfluence = (1.0f - (Distance / 300.0f)) * 25.0f * Feature.Intensity;
                }
                break;
                
            default:
                break;
        }
        
        Temperature += HeatInfluence;
    }
    
    // Add heat from lava flows
    for (const FWorld_LavaFlowData& Flow : LavaFlows)
    {
        if (!Flow.bIsFlowing)
            continue;
            
        for (const FVector& FlowPoint : Flow.FlowPath)
        {
            float Distance = FVector::Dist(Location, FlowPoint);
            if (Distance < Flow.Width * 1.5f)
            {
                float HeatInfluence = (1.0f - (Distance / (Flow.Width * 1.5f))) * 40.0f;
                Temperature += HeatInfluence;
            }
        }
    }
    
    return Temperature;
}

bool UWorld_VolcanicBiomeSystem::IsLocationTooHot(const FVector& Location, float Threshold) const
{
    return GetTemperatureAtLocation(Location) > Threshold;
}

void UWorld_VolcanicBiomeSystem::UpdateTemperatureField()
{
    // This could be expanded to update a temperature grid for performance
    // For now, we calculate temperature on-demand in GetTemperatureAtLocation
}

void UWorld_VolcanicBiomeSystem::InitializeVolcanicAssets()
{
    // Asset loading would typically be done via Blueprint or asset references
    // This is a placeholder for asset initialization
    UE_LOG(LogTemp, Warning, TEXT("VolcanicBiomeSystem: Initializing volcanic assets"));
}

void UWorld_VolcanicBiomeSystem::CleanupSpawnedComponents()
{
    // Clean up mesh components
    for (UStaticMeshComponent* MeshComp : SpawnedMeshComponents)
    {
        if (IsValid(MeshComp) && IsValid(MeshComp->GetOwner()))
        {
            MeshComp->GetOwner()->Destroy();
        }
    }
    SpawnedMeshComponents.Empty();
    
    // Clean up particle components
    for (UParticleSystemComponent* ParticleComp : SpawnedParticleComponents)
    {
        if (IsValid(ParticleComp))
        {
            ParticleComp->DestroyComponent();
        }
    }
    SpawnedParticleComponents.Empty();
    
    // Clean up audio components
    for (UAudioComponent* AudioComp : SpawnedAudioComponents)
    {
        if (IsValid(AudioComp))
        {
            AudioComp->Stop();
            AudioComp->DestroyComponent();
        }
    }
    SpawnedAudioComponents.Empty();
}

FVector UWorld_VolcanicBiomeSystem::FindSuitableVolcanicLocation(const FVector& CenterLocation, float SearchRadius) const
{
    // Try up to 10 random locations
    for (int32 Attempt = 0; Attempt < 10; Attempt++)
    {
        FVector TestLocation = CenterLocation + FVector(
            FMath::RandRange(-SearchRadius, SearchRadius),
            FMath::RandRange(-SearchRadius, SearchRadius),
            0
        );
        
        if (IsValidVolcanicSpawnLocation(TestLocation))
        {
            return TestLocation;
        }
    }
    
    return FVector::ZeroVector; // No suitable location found
}

bool UWorld_VolcanicBiomeSystem::IsValidVolcanicSpawnLocation(const FVector& Location) const
{
    // Check if location is too close to existing features
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        float MinDistance = Feature.Radius + 200.0f; // Minimum spacing
        
        if (Distance < MinDistance)
        {
            return false;
        }
    }
    
    return true;
}

float UWorld_VolcanicBiomeSystem::CalculateHeatIntensity(const FVector& Location) const
{
    float HeatIntensity = 0.0f;
    
    for (const FWorld_VolcanicFeature& Feature : VolcanicFeatures)
    {
        float Distance = FVector::Dist(Location, Feature.Location);
        if (Distance < Feature.Radius * 2.0f)
        {
            float Influence = (1.0f - (Distance / (Feature.Radius * 2.0f))) * Feature.Intensity;
            HeatIntensity += Influence;
        }
    }
    
    return FMath::Clamp(HeatIntensity, 0.0f, 1.0f);
}

void UWorld_VolcanicBiomeSystem::UpdateLavaFlowAnimation(float DeltaTime)
{
    // Update lava flow animations
    for (FWorld_LavaFlowData& Flow : LavaFlows)
    {
        if (Flow.bIsFlowing)
        {
            // This could animate lava flow progression
            // For now, just ensure flows remain active based on volcanic activity
            Flow.bIsFlowing = (VolcanicActivityLevel > 0.2f);
        }
    }
}

void UWorld_VolcanicBiomeSystem::UpdateParticleEffects(float DeltaTime)
{
    // Update particle system intensities based on volcanic activity
    for (UParticleSystemComponent* ParticleComp : SpawnedParticleComponents)
    {
        if (IsValid(ParticleComp))
        {
            // Scale particle effects with volcanic activity
            FVector Scale = FVector(VolcanicActivityLevel);
            ParticleComp->SetWorldScale3D(Scale);
        }
    }
}