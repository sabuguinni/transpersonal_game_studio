#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "ArchitecturalManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    None            UMETA(DisplayName = "None"),
    StoneRuins      UMETA(DisplayName = "Stone Ruins"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockFormation   UMETA(DisplayName = "Rock Formation"),
    AncientPillar   UMETA(DisplayName = "Ancient Pillar"),
    TribalShelter   UMETA(DisplayName = "Tribal Shelter")
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsInteractable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::None;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Savana;
        bIsInteractable = false;
        StructuralIntegrity = 100.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArch_StructureData> StructureDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EBiomeType, int32> StructuresPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinDistanceBetweenStructures = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bAutoSpawnStructures = true;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, FRotator Rotation, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void PopulateBiomeWithStructures(EBiomeType BiomeType, int32 StructureCount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ClearStructuresInBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Architecture", CallInEditor)
    void GenerateAllBiomeStructures();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationSuitableForStructure(FVector Location, EArch_StructureType StructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType);

private:
    UPROPERTY()
    TArray<AActor*> SpawnedStructures;

    void InitializeStructureDatabase();
    AActor* CreateStructureActor(EArch_StructureType StructureType, FVector Location, FRotator Rotation);
    FString GetStructureMeshPath(EArch_StructureType StructureType);
};