#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "ArchitectureSystem.generated.h"

/**
 * Enumeração dos tipos de estruturas arquitectónicas
 */
UENUM(BlueprintType)
enum class EArchitectureType : uint8
{
    EmergencyShelter    UMETA(DisplayName = "Emergency Shelter"),
    SemiPermanentHut    UMETA(DisplayName = "Semi-Permanent Hut"),
    DefensiveStructure  UMETA(DisplayName = "Defensive Structure"),
    CommunityBuilding   UMETA(DisplayName = "Community Building"),
    AncientRuin         UMETA(DisplayName = "Ancient Ruin")
};

/**
 * Enumeração dos materiais de construção disponíveis
 */
UENUM(BlueprintType)
enum class EBuildingMaterial : uint8
{
    Wood        UMETA(DisplayName = "Wood"),
    Stone       UMETA(DisplayName = "Stone"),
    Plant       UMETA(DisplayName = "Plant Fibers"),
    Bone        UMETA(DisplayName = "Dinosaur Bone"),
    Clay        UMETA(DisplayName = "Clay/Mud")
};

/**
 * Estado de conservação da estrutura
 */
UENUM(BlueprintType)
enum class EStructureCondition : uint8
{
    Perfect     UMETA(DisplayName = "Perfect"),
    Good        UMETA(DisplayName = "Good"),
    Damaged     UMETA(DisplayName = "Damaged"),
    Ruined      UMETA(DisplayName = "Ruined"),
    Collapsed   UMETA(DisplayName = "Collapsed")
};

/**
 * Estrutura de dados para configuração de edifícios
 */
USTRUCT(BlueprintType)
struct FArchitectureConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArchitectureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EBuildingMaterial> RequiredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ConstructionTimeHours;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxOccupancy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefenseRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasMultipleExits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasStorage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasFireplace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> PrimaryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UStaticMesh>> DetailMeshes;
};

/**
 * Componente que define detalhes ambientais de uma estrutura
 */
USTRUCT(BlueprintType)
struct FEnvironmentalDetails
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasExtinguishedFireplace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAbandonedTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasFoodRemains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasFootprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWallMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasInvasiveVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasAnimalDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasForgottenItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AbandonmentLevel; // 0.0 = recently used, 1.0 = long abandoned
};

/**
 * Actor principal do sistema de arquitectura
 */
UCLASS()
class TRANSPERSONALGAME_API AArchitectureSystem : public AActor
{
    GENERATED_BODY()

public:
    AArchitectureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArchitectureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EStructureCondition Condition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEnvironmentalDetails EnvironmentalDetails;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    UDataTable* ArchitectureConfigTable;

public:
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArchitectureType Type, EStructureCondition InitialCondition);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyEnvironmentalDetails();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetCondition(EStructureCondition NewCondition);

    UFUNCTION(BlueprintPure, Category = "Architecture")
    bool CanPlayerEnter() const;

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetDefenseRating() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnPlayerEntered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnPlayerExited();

private:
    void LoadStructureConfiguration();
    void SpawnDetailMeshes();
    void ApplyWeatheringEffects();
};