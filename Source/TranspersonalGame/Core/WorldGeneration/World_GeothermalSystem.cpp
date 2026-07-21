#include "World_GeothermalSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UWorld_GeothermalSystem::UWorld_GeothermalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;

    // Initialize default values
    WorldBounds = 50000.0f;
    MinVentIntensity = 0.5f;
    MaxVentIntensity = 2.0f;
    MinSpringTemperature = 35.0f;
    MaxSpringTemperature = 65.0f;

    // Initialize asset pointers to null
    VentMesh = nullptr;
    HotSpringMesh = nullptr;
    SteamParticleSystem = nullptr;
    GeothermalAmbientSound = nullptr;
    HotSpringBubblingSound = nullptr;
    HotWaterMaterial = nullptr;
    GeothermalRockMaterial = nullptr;
}

void UWorld_GeothermalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultAssets();
    
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: BeginPlay - System initialized"));
}

void UWorld_GeothermalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateGeothermalEffects(DeltaTime);
}

void UWorld_GeothermalSystem::GenerateGeothermalVents(int32 NumVents, float MinDistance)
{
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Generating %d geothermal vents"), NumVents);
    
    for (int32 i = 0; i < NumVents; i++)
    {
        FVector VentLocation = FindValidGeothermalLocation(MinDistance);
        if (VentLocation != FVector::ZeroVector)
        {
            float Intensity = FMath::RandRange(MinVentIntensity, MaxVentIntensity);
            CreateGeothermalVent(VentLocation, Intensity);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Generated %d geothermal vents"), GeothermalVents.Num());
}

void UWorld_GeothermalSystem::CreateGeothermalVent(const FVector& Location, float Intensity)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_GeothermalSystem: No valid world for geothermal vent creation"));
        return;
    }

    // Create vent data
    FWorld_GeothermalVentData VentData;
    VentData.Location = Location;
    VentData.Intensity = Intensity;
    VentData.Temperature = FMath::RandRange(70.0f, 95.0f);
    VentData.Radius = FMath::RandRange(300.0f, 800.0f);
    VentData.bIsActive = true;

    GeothermalVents.Add(VentData);

    // Spawn visual representation
    if (VentMesh)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* VentActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (VentActor)
        {
            UStaticMeshComponent* MeshComp = VentActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(VentMesh);
                if (GeothermalRockMaterial)
                {
                    MeshComp->SetMaterial(0, GeothermalRockMaterial);
                }
                
                // Scale based on intensity
                float Scale = 0.8f + (Intensity * 0.4f);
                VentActor->SetActorScale3D(FVector(Scale));
            }
            
            VentActor->SetActorLabel(FString::Printf(TEXT("GeothermalVent_%d"), GeothermalVents.Num()));
            SpawnedVentActors.Add(VentActor);
        }
    }

    // Spawn steam effects
    SpawnSteamEffects(Location, Intensity);

    // Add ambient sound
    if (GeothermalAmbientSound)
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            World, 
            GeothermalAmbientSound, 
            Location,
            FRotator::ZeroRotator,
            Intensity * 0.7f
        );
        
        if (AudioComp)
        {
            ActiveAudioSources.Add(AudioComp);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Created geothermal vent at %s with intensity %.2f"), 
           *Location.ToString(), Intensity);
}

void UWorld_GeothermalSystem::RemoveGeothermalVent(int32 VentIndex)
{
    if (GeothermalVents.IsValidIndex(VentIndex))
    {
        GeothermalVents.RemoveAt(VentIndex);
        
        if (SpawnedVentActors.IsValidIndex(VentIndex) && SpawnedVentActors[VentIndex])
        {
            SpawnedVentActors[VentIndex]->Destroy();
            SpawnedVentActors.RemoveAt(VentIndex);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Removed geothermal vent at index %d"), VentIndex);
    }
}

void UWorld_GeothermalSystem::GenerateHotSprings(int32 NumSprings, float MinDistance)
{
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Generating %d hot springs"), NumSprings);
    
    for (int32 i = 0; i < NumSprings; i++)
    {
        FVector SpringLocation = FindValidGeothermalLocation(MinDistance);
        if (SpringLocation != FVector::ZeroVector)
        {
            float Temperature = FMath::RandRange(MinSpringTemperature, MaxSpringTemperature);
            CreateHotSpring(SpringLocation, Temperature);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Generated %d hot springs"), HotSprings.Num());
}

void UWorld_GeothermalSystem::CreateHotSpring(const FVector& Location, float Temperature)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_GeothermalSystem: No valid world for hot spring creation"));
        return;
    }

    // Create spring data
    FWorld_HotSpringData SpringData;
    SpringData.Location = Location;
    SpringData.WaterTemperature = Temperature;
    SpringData.PoolRadius = FMath::RandRange(200.0f, 500.0f);
    SpringData.PoolDepth = FMath::RandRange(100, 300);
    SpringData.bHasSteam = Temperature > 40.0f;

    HotSprings.Add(SpringData);

    // Spawn visual representation
    if (HotSpringMesh)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        AStaticMeshActor* SpringActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
        if (SpringActor)
        {
            UStaticMeshComponent* MeshComp = SpringActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                MeshComp->SetStaticMesh(HotSpringMesh);
                if (HotWaterMaterial)
                {
                    MeshComp->SetMaterial(0, HotWaterMaterial);
                }
                
                // Scale based on pool radius
                float Scale = SpringData.PoolRadius / 300.0f;
                SpringActor->SetActorScale3D(FVector(Scale, Scale, 1.0f));
            }
            
            SpringActor->SetActorLabel(FString::Printf(TEXT("HotSpring_%d"), HotSprings.Num()));
            SpawnedSpringActors.Add(SpringActor);
        }
    }

    // Spawn steam if temperature is high enough
    if (SpringData.bHasSteam)
    {
        SpawnSteamEffects(Location, Temperature / 65.0f);
    }

    // Add bubbling sound
    if (HotSpringBubblingSound)
    {
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSoundAtLocation(
            World, 
            HotSpringBubblingSound, 
            Location,
            FRotator::ZeroRotator,
            0.6f
        );
        
        if (AudioComp)
        {
            ActiveAudioSources.Add(AudioComp);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Created hot spring at %s with temperature %.1f°C"), 
           *Location.ToString(), Temperature);
}

void UWorld_GeothermalSystem::RemoveHotSpring(int32 SpringIndex)
{
    if (HotSprings.IsValidIndex(SpringIndex))
    {
        HotSprings.RemoveAt(SpringIndex);
        
        if (SpawnedSpringActors.IsValidIndex(SpringIndex) && SpawnedSpringActors[SpringIndex])
        {
            SpawnedSpringActors[SpringIndex]->Destroy();
            SpawnedSpringActors.RemoveAt(SpringIndex);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Removed hot spring at index %d"), SpringIndex);
    }
}

void UWorld_GeothermalSystem::UpdateGeothermalEffects(float DeltaTime)
{
    // Update steam effects intensity based on time and environmental factors
    for (int32 i = 0; i < ActiveSteamEffects.Num(); i++)
    {
        if (ActiveSteamEffects[i] && IsValid(ActiveSteamEffects[i]))
        {
            // Vary steam intensity slightly over time
            float TimeVariation = FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f + i) * 0.2f + 1.0f;
            ActiveSteamEffects[i]->SetFloatParameter(TEXT("Intensity"), TimeVariation);
        }
        else
        {
            // Clean up invalid components
            ActiveSteamEffects.RemoveAt(i);
            i--;
        }
    }
}

void UWorld_GeothermalSystem::SpawnSteamEffects(const FVector& Location, float Intensity)
{
    UWorld* World = GetWorld();
    if (!World || !SteamParticleSystem)
    {
        return;
    }

    UParticleSystemComponent* SteamComponent = UGameplayStatics::SpawnEmitterAtLocation(
        World,
        SteamParticleSystem,
        Location + FVector(0, 0, 50),
        FRotator::ZeroRotator,
        FVector(Intensity),
        true
    );

    if (SteamComponent)
    {
        SteamComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
        ActiveSteamEffects.Add(SteamComponent);
    }
}

void UWorld_GeothermalSystem::UpdateHotSpringEffects(int32 SpringIndex)
{
    if (!HotSprings.IsValidIndex(SpringIndex))
    {
        return;
    }

    const FWorld_HotSpringData& SpringData = HotSprings[SpringIndex];
    
    // Update steam effects if temperature changed
    if (SpringData.bHasSteam)
    {
        float SteamIntensity = SpringData.WaterTemperature / 65.0f;
        SpawnSteamEffects(SpringData.Location, SteamIntensity);
    }
}

float UWorld_GeothermalSystem::GetTemperatureAtLocation(const FVector& Location) const
{
    float BaseTemperature = 25.0f; // Base environmental temperature
    float MaxInfluence = 0.0f;

    // Check influence from geothermal vents
    for (const FWorld_GeothermalVentData& Vent : GeothermalVents)
    {
        if (Vent.bIsActive)
        {
            float Distance = FVector::Dist(Location, Vent.Location);
            if (Distance < Vent.Radius)
            {
                float Influence = (1.0f - Distance / Vent.Radius) * Vent.Intensity;
                float TemperatureIncrease = Influence * (Vent.Temperature - BaseTemperature);
                MaxInfluence = FMath::Max(MaxInfluence, TemperatureIncrease);
            }
        }
    }

    // Check influence from hot springs
    for (const FWorld_HotSpringData& Spring : HotSprings)
    {
        float Distance = FVector::Dist(Location, Spring.Location);
        if (Distance < Spring.PoolRadius * 2.0f)
        {
            float Influence = (1.0f - Distance / (Spring.PoolRadius * 2.0f));
            float TemperatureIncrease = Influence * (Spring.WaterTemperature - BaseTemperature) * 0.5f;
            MaxInfluence = FMath::Max(MaxInfluence, TemperatureIncrease);
        }
    }

    return BaseTemperature + MaxInfluence;
}

bool UWorld_GeothermalSystem::IsLocationNearGeothermalActivity(const FVector& Location, float CheckRadius) const
{
    // Check proximity to geothermal vents
    for (const FWorld_GeothermalVentData& Vent : GeothermalVents)
    {
        if (Vent.bIsActive)
        {
            float Distance = FVector::Dist(Location, Vent.Location);
            if (Distance < CheckRadius)
            {
                return true;
            }
        }
    }

    // Check proximity to hot springs
    for (const FWorld_HotSpringData& Spring : HotSprings)
    {
        float Distance = FVector::Dist(Location, Spring.Location);
        if (Distance < CheckRadius)
        {
            return true;
        }
    }

    return false;
}

void UWorld_GeothermalSystem::ApplyGeothermalEnvironmentalEffects()
{
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Applying environmental effects for %d vents and %d springs"), 
           GeothermalVents.Num(), HotSprings.Num());
    
    // This function can be extended to apply environmental effects like:
    // - Increased humidity near hot springs
    // - Mineral deposits around vents
    // - Unique vegetation patterns
    // - Wildlife behavior changes
}

void UWorld_GeothermalSystem::ClearAllGeothermalFeatures()
{
    // Clean up spawned actors
    CleanupGeothermalActors();
    
    // Clear data arrays
    GeothermalVents.Empty();
    HotSprings.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Cleared all geothermal features"));
}

int32 UWorld_GeothermalSystem::GetActiveVentCount() const
{
    int32 ActiveCount = 0;
    for (const FWorld_GeothermalVentData& Vent : GeothermalVents)
    {
        if (Vent.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

int32 UWorld_GeothermalSystem::GetActiveSpringCount() const
{
    return HotSprings.Num(); // All springs are considered active
}

FVector UWorld_GeothermalSystem::FindValidGeothermalLocation(float MinDistance) const
{
    const int32 MaxAttempts = 50;
    
    for (int32 Attempt = 0; Attempt < MaxAttempts; Attempt++)
    {
        FVector TestLocation = FVector(
            FMath::RandRange(-WorldBounds, WorldBounds),
            FMath::RandRange(-WorldBounds, WorldBounds),
            0.0f
        );
        
        if (IsLocationValidForGeothermal(TestLocation, MinDistance))
        {
            return TestLocation;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Could not find valid location after %d attempts"), MaxAttempts);
    return FVector::ZeroVector;
}

bool UWorld_GeothermalSystem::IsLocationValidForGeothermal(const FVector& Location, float MinDistance) const
{
    // Check distance from existing geothermal vents
    for (const FWorld_GeothermalVentData& Vent : GeothermalVents)
    {
        if (FVector::Dist(Location, Vent.Location) < MinDistance)
        {
            return false;
        }
    }
    
    // Check distance from existing hot springs
    for (const FWorld_HotSpringData& Spring : HotSprings)
    {
        if (FVector::Dist(Location, Spring.Location) < MinDistance)
        {
            return false;
        }
    }
    
    return true;
}

void UWorld_GeothermalSystem::CleanupGeothermalActors()
{
    // Destroy vent actors
    for (AActor* Actor : SpawnedVentActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedVentActors.Empty();
    
    // Destroy spring actors
    for (AActor* Actor : SpawnedSpringActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedSpringActors.Empty();
    
    // Clean up particle effects
    for (UParticleSystemComponent* Effect : ActiveSteamEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DestroyComponent();
        }
    }
    ActiveSteamEffects.Empty();
    
    // Clean up audio components
    for (UAudioComponent* Audio : ActiveAudioSources)
    {
        if (Audio && IsValid(Audio))
        {
            Audio->Stop();
            Audio->DestroyComponent();
        }
    }
    ActiveAudioSources.Empty();
}

void UWorld_GeothermalSystem::InitializeDefaultAssets()
{
    // In a real implementation, these would load actual assets
    // For now, we'll use basic engine assets or leave as null
    UE_LOG(LogTemp, Warning, TEXT("World_GeothermalSystem: Initializing default assets - implement asset loading"));
}