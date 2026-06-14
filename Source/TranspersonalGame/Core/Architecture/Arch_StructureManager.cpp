#include "Arch_StructureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UArch_StructureManager::UArch_StructureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    StructureDetectionRange = 1000.0f;
    bAutoDetectStructures = true;
}

void UArch_StructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoDetectStructures)
    {
        // Auto-detect existing structures in the world
        UWorld* World = GetWorld();
        if (World)
        {
            // Find trigger volumes that represent structure interiors
            TArray<AActor*> FoundTriggers;
            UGameplayStatics::GetAllActorsOfClass(World, ATriggerVolume::StaticClass(), FoundTriggers);
            
            for (AActor* TriggerActor : FoundTriggers)
            {
                FString ActorLabel = TriggerActor->GetActorLabel();
                if (ActorLabel.Contains("Dwelling") || ActorLabel.Contains("Structure") || ActorLabel.Contains("Interior"))
                {
                    FArch_StructureData NewStructure;
                    NewStructure.Location = TriggerActor->GetActorLocation();
                    
                    if (ActorLabel.Contains("Dwelling"))
                        NewStructure.StructureType = EArch_StructureType::Dwelling;
                    else if (ActorLabel.Contains("Ruins"))
                        NewStructure.StructureType = EArch_StructureType::Ruins;
                    else if (ActorLabel.Contains("Shelter"))
                        NewStructure.StructureType = EArch_StructureType::Shelter;
                    
                    RegisteredStructures.Add(NewStructure);
                }
            }
        }
    }
}

void UArch_StructureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structure states periodically
    for (int32 i = 0; i < RegisteredStructures.Num(); i++)
    {
        FArch_StructureData& Structure = RegisteredStructures[i];
        
        // Simulate gradual decay for ruins
        if (Structure.StructureType == EArch_StructureType::Ruins)
        {
            Structure.Integrity = FMath::Max(0.0f, Structure.Integrity - (DeltaTime * 0.1f));
        }
    }
}

void UArch_StructureManager::RegisterStructure(const FArch_StructureData& StructureData)
{
    RegisteredStructures.Add(StructureData);
}

void UArch_StructureManager::UnregisterStructure(int32 StructureIndex)
{
    if (RegisteredStructures.IsValidIndex(StructureIndex))
    {
        RegisteredStructures.RemoveAt(StructureIndex);
    }
}

TArray<FArch_StructureData> UArch_StructureManager::GetNearbyStructures(FVector PlayerLocation, float SearchRadius)
{
    TArray<FArch_StructureData> NearbyStructures;
    
    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        float Distance = FVector::Dist(PlayerLocation, Structure.Location);
        if (Distance <= SearchRadius)
        {
            NearbyStructures.Add(Structure);
        }
    }
    
    return NearbyStructures;
}

bool UArch_StructureManager::IsInsideStructure(FVector Location)
{
    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        float Distance = FVector::Dist(Location, Structure.Location);
        if (Distance <= 300.0f) // Interior radius
        {
            return true;
        }
    }
    
    return false;
}

void UArch_StructureManager::UpdateStructureIntegrity(int32 StructureIndex, float NewIntegrity)
{
    if (RegisteredStructures.IsValidIndex(StructureIndex))
    {
        RegisteredStructures[StructureIndex].Integrity = FMath::Clamp(NewIntegrity, 0.0f, 100.0f);
    }
}

void UArch_StructureManager::SetStructureOccupancy(int32 StructureIndex, bool bOccupied)
{
    if (RegisteredStructures.IsValidIndex(StructureIndex))
    {
        RegisteredStructures[StructureIndex].bIsOccupied = bOccupied;
    }
}

FArch_StructureData UArch_StructureManager::GetStructureAtLocation(FVector Location, float Tolerance)
{
    for (const FArch_StructureData& Structure : RegisteredStructures)
    {
        float Distance = FVector::Dist(Location, Structure.Location);
        if (Distance <= Tolerance)
        {
            return Structure;
        }
    }
    
    // Return empty structure if none found
    return FArch_StructureData();
}