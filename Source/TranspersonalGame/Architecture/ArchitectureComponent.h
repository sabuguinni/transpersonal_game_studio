#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ArchitectureTypes.h"
#include "ArchitectureComponent.generated.h"

/**
 * Core component for all architectural structures in the game.
 * Handles structure data, materials, aging, and interior systems.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_ArchitectureComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ArchitectureComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Structure Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_InteriorData InteriorData;

    // Mesh and Material References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Visuals")
    TSoftObjectPtr<UStaticMesh> StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Visuals")
    TSoftObjectPtr<UMaterialInterface> PrimaryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Visuals")
    TSoftObjectPtr<UMaterialInterface> SecondaryMaterial;

    // Aging and Weather System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Aging")
    float AgingRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Aging")
    float WeatherResistance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Aging")
    bool bEnableAging = true;

    // Interior System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Interior")
    bool bHasLoadableInterior = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Interior")
    FString InteriorLevelName;

    // Interaction System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Interaction")
    bool bCanEnter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Interaction")
    float InteractionDistance = 200.0f;

    // Blueprint Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureEntered();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureExited();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnStructureAged(float NewAge);

    // Public Interface Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureCondition(EArch_StructureCondition NewCondition);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void AgeStructure(float AgeAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void RepairStructure(float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanPlayerEnter() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void EnterStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ExitStructure();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_StructureData GetStructureData() const { return StructureData; }

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_InteriorData GetInteriorData() const { return InteriorData; }

protected:
    // Internal Functions
    void UpdateStructureAppearance();
    void UpdateMaterials();
    void ProcessAging(float DeltaTime);
    void UpdateInteriorConditions();

private:
    // Cached references
    UPROPERTY()
    UStaticMeshComponent* CachedMeshComponent;

    // Runtime state
    float LastAgingUpdate;
    bool bIsPlayerInside;
};