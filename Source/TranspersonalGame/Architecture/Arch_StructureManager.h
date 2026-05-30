#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    TribalShelter   UMETA(DisplayName = "Tribal Shelter"),
    AncientRuins    UMETA(DisplayName = "Ancient Ruins")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savana;

    FArch_StructureSpawnData()
    {
        StructureType = EArch_StructureType::StonePillar;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savana;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Structure spawning and management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtBiome(EArch_StructureType StructureType, EBiomeType BiomeType, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(const FArch_StructureSpawnData& SpawnData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void DistributeStructuresAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearAllStructures();

    // Biome coordinate system
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetBiomeCenter(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType, float Radius = 15000.0f);

protected:
    // Structure tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TArray<AActor*> SpawnedStructures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EArch_StructureType, TSoftObjectPtr<UStaticMesh>> StructureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoDistributeOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 StructuresPerBiome = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures = 2000.0f;

private:
    // Internal spawning logic
    AActor* SpawnStructureMesh(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    bool IsLocationValid(const FVector& Location, float MinDistance);
    UStaticMesh* GetStructureMesh(EArch_StructureType StructureType);
    void InitializeStructureMeshes();
};