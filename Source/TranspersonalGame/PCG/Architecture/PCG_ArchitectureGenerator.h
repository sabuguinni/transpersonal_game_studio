// PCG_ArchitectureGenerator.h
// Transpersonal Game Studio - Architecture & Interior Agent
// Procedural generation system for prehistoric structures

#pragma once

#include "CoreMinimal.h"
#include "PCGElement.h"
#include "PCGSettings.h"
#include "Engine/DataTable.h"
#include "PCG_ArchitectureGenerator.generated.h"

UENUM(BlueprintType)
enum class EArchitectureType : uint8
{
    EmergencyShelter    UMETA(DisplayName = "Emergency Shelter"),
    TemporaryHabitat    UMETA(DisplayName = "Temporary Habitat"),
    PermanentBase       UMETA(DisplayName = "Permanent Base"),
    UtilityStructure    UMETA(DisplayName = "Utility Structure"),
    Ruins               UMETA(DisplayName = "Ruins"),
    AbandonedSite       UMETA(DisplayName = "Abandoned Site")
};

UENUM(BlueprintType)
enum class EStructuralCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    WellMaintained  UMETA(DisplayName = "Well Maintained"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Deteriorating   UMETA(DisplayName = "Deteriorating"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct FArchitectureSpawnData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArchitectureType StructureType = EArchitectureType::EmergencyShelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<TSoftObjectPtr<UStaticMesh>> PossibleMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector MinScale = FVector(0.8f, 0.8f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector MaxScale = FVector(1.2f, 1.2f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EStructuralCondition Condition = EStructuralCondition::WellMaintained;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bRequiresWaterProximity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bRequiresRockFormation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bRequiresTreeCover = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MinSlopeAngle = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MaxSlopeAngle = 30.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPCG_ArchitectureGeneratorSettings : public UPCGSettings
{
    GENERATED_BODY()

public:
    UPCG_ArchitectureGeneratorSettings();

    // Architecture generation parameters
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    UDataTable* ArchitectureDataTable;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StructureDensity = 0.1f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    float MinDistanceBetweenStructures = 500.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    bool bGenerateInteriorDetails = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    bool bGenerateDefensiveElements = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Architecture Generation")
    bool bGenerateStorageAreas = true;

    // Environmental constraints
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Constraints")
    float WaterProximityRange = 1000.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Constraints")
    float RockFormationRange = 200.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Environmental Constraints")
    float TreeCoverRange = 100.0f;

    // Storytelling elements
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storytelling")
    bool bGenerateAbandonmentSigns = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storytelling")
    bool bGeneratePersonalItems = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storytelling")
    bool bGenerateDefensiveDamage = true;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Storytelling")
    float AbandonmentProbability = 0.3f;

protected:
    //~Begin UPCGSettings interface
#if WITH_EDITOR
    virtual FText GetDefaultNodeTitle() const override;
    virtual FText GetNodeTooltipText() const override;
    virtual FLinearColor GetNodeTitleColor() const override;
#endif
    virtual TArray<FPCGPinProperties> InputPinProperties() const override;
    virtual TArray<FPCGPinProperties> OutputPinProperties() const override;
    virtual UPCGNode* CreateNode() const override;
    //~End UPCGSettings interface

public:
    virtual FPCGElementPtr CreateElement() const override;
};

class TRANSPERSONALGAME_API FPCGArchitectureGeneratorElement : public IPCGElement
{
public:
    virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;
    virtual bool ExecuteInternal(FPCGContext* Context) const override;

protected:
    bool GenerateArchitecturePoints(FPCGContext* Context, const UPCGPointData* InPointData, UPCGPointData* OutPointData) const;
    bool ValidateSpawnLocation(const FPCGPoint& Point, const FArchitectureSpawnData& SpawnData, FPCGContext* Context) const;
    void ApplyArchitectureAttributes(FPCGPoint& Point, const FArchitectureSpawnData& SpawnData) const;
    void GenerateInteriorElements(FPCGContext* Context, const FPCGPoint& StructurePoint, UPCGPointData* OutPointData) const;
    void GenerateStorytellingElements(FPCGContext* Context, const FPCGPoint& StructurePoint, UPCGPointData* OutPointData) const;
};