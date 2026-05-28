#include "VFX_FootstepEffectManager.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UVFX_FootstepEffectManager::UVFX_FootstepEffectManager()
{
    BaseDustScale = 1.0f;
    MaxDustLifetime = 3.0f;
    WeightMultiplier = 0.001f;
}

void UVFX_FootstepEffectManager::TriggerFootstepEffect(const FVFX_FootstepData& FootstepData)
{
    if (!IsValid(this))
    {
        return;
    }

    // Spawn dust particles based on terrain type
    SpawnDustParticles(FootstepData);

    // Create ground crack for heavy creatures
    if (FootstepData.CreatureWeight > 5000.0f)
    {
        CreateGroundCrack(FootstepData);
    }

    // Play footstep sound
    PlayFootstepSound(FootstepData);

    UE_LOG(LogTemp, Log, TEXT("VFX Footstep triggered at %s, Weight: %f, Terrain: %d"), 
           *FootstepData.ImpactLocation.ToString(), 
           FootstepData.CreatureWeight, 
           (int32)FootstepData.TerrainType);
}

void UVFX_FootstepEffectManager::TriggerDinosaurFootstep(FVector Location, float Weight, EVFX_TerrainType Terrain)
{
    FVFX_FootstepData FootstepData;
    FootstepData.ImpactLocation = Location;
    FootstepData.CreatureWeight = Weight;
    FootstepData.TerrainType = Terrain;
    FootstepData.DustIntensity = FMath::Clamp(Weight * WeightMultiplier, 0.5f, 5.0f);

    TriggerFootstepEffect(FootstepData);
}

void UVFX_FootstepEffectManager::TriggerPlayerFootstep(FVector Location, EVFX_TerrainType Terrain)
{
    FVFX_FootstepData FootstepData;
    FootstepData.ImpactLocation = Location;
    FootstepData.CreatureWeight = 70.0f; // Average human weight
    FootstepData.TerrainType = Terrain;
    FootstepData.DustIntensity = 0.3f; // Light dust for human footsteps

    TriggerFootstepEffect(FootstepData);
}

EVFX_TerrainType UVFX_FootstepEffectManager::DetectTerrainType(FVector Location)
{
    // Simple terrain detection based on location
    // In a real implementation, this would use surface materials or terrain layers
    
    if (Location.X > 50000.0f) // Desert biome
    {
        return EVFX_TerrainType::Sand;
    }
    else if (Location.X < -40000.0f && Location.Y > 35000.0f) // Forest biome
    {
        return EVFX_TerrainType::Mud;
    }
    else if (Location.Z > 1000.0f) // Mountain biome
    {
        return EVFX_TerrainType::Rock;
    }
    else if (Location.X < -45000.0f && Location.Y < -40000.0f) // Swamp biome
    {
        return EVFX_TerrainType::Mud;
    }
    
    return EVFX_TerrainType::Dirt; // Default Savana terrain
}

void UVFX_FootstepEffectManager::SpawnDustParticles(const FVFX_FootstepData& Data)
{
    // Create dust particle effect based on terrain type
    FVector SpawnLocation = Data.ImpactLocation + FVector(0, 0, 10);
    float DustScale = BaseDustScale * Data.DustIntensity;

    // Log dust particle creation
    UE_LOG(LogTemp, Log, TEXT("Spawning dust particles at %s, Scale: %f, Terrain: %d"), 
           *SpawnLocation.ToString(), DustScale, (int32)Data.TerrainType);

    // In a real implementation, this would spawn actual particle systems
    // For now, we create debug visualization
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (World)
    {
        // Draw debug sphere to represent dust cloud
        DrawDebugSphere(World, SpawnLocation, DustScale * 50.0f, 12, FColor::Brown, false, MaxDustLifetime);
        
        // Create multiple dust particles around impact point
        for (int32 i = 0; i < 5; i++)
        {
            FVector ParticleOffset = FVector(
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(0.0f, 50.0f)
            );
            DrawDebugPoint(World, SpawnLocation + ParticleOffset, 5.0f, FColor::Orange, false, MaxDustLifetime);
        }
    }
}

void UVFX_FootstepEffectManager::CreateGroundCrack(const FVFX_FootstepData& Data)
{
    // Create ground crack effect for heavy dinosaurs
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (World)
    {
        FVector CrackStart = Data.ImpactLocation;
        float CrackLength = FMath::Clamp(Data.CreatureWeight * 0.01f, 50.0f, 300.0f);
        
        // Draw radiating cracks
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) + FMath::RandRange(-15.0f, 15.0f);
            FVector CrackDirection = FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)),
                FMath::Sin(FMath::DegreesToRadians(Angle)),
                0
            );
            FVector CrackEnd = CrackStart + (CrackDirection * CrackLength);
            
            DrawDebugLine(World, CrackStart, CrackEnd, FColor::Black, false, MaxDustLifetime * 2.0f, 0, 3.0f);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Ground crack created at %s, Length: %f"), 
               *CrackStart.ToString(), CrackLength);
    }
}

void UVFX_FootstepEffectManager::PlayFootstepSound(const FVFX_FootstepData& Data)
{
    // Play appropriate footstep sound based on terrain type
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (World)
    {
        // Log sound effect (in real implementation would play actual audio)
        FString TerrainName;
        switch (Data.TerrainType)
        {
            case EVFX_TerrainType::Dirt: TerrainName = TEXT("Dirt"); break;
            case EVFX_TerrainType::Sand: TerrainName = TEXT("Sand"); break;
            case EVFX_TerrainType::Rock: TerrainName = TEXT("Rock"); break;
            case EVFX_TerrainType::Mud: TerrainName = TEXT("Mud"); break;
            case EVFX_TerrainType::Snow: TerrainName = TEXT("Snow"); break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Playing %s footstep sound at %s, Weight: %f"), 
               *TerrainName, *Data.ImpactLocation.ToString(), Data.CreatureWeight);
    }
}