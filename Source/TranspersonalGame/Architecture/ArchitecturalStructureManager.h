#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "ArchitecturalStructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling    UMETA(DisplayName = "Cave Dwelling"),
    WoodenShelter   UMETA(DisplayName = "Wooden Shelter"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    Fortification   UMETA(DisplayName = "Fortification"),
    StorageHut      UMETA(DisplayName = "Storage Hut"),
    Watchtower      UMETA(DisplayName = "Watchtower")
};

UENUM(BlueprintType)
enum class EArch_StructureState : uint8
{
    Intact          UMETA(DisplayName = "Intact"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruins           UMETA(DisplayName = "Ruins"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureState CurrentState = EArch_StructureState::Intact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInteriorSpace = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsDefensive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float DefenseRating = 0.0f;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        CurrentState = EArch_StructureState::Intact;
        StructuralIntegrity = 100.0f;
        MaxOccupants = 4;
        bHasInteriorSpace = true;
        bIsDefensive = false;
        DefenseRating = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArchitecturalStructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArchitecturalStructureManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionBounds;

    // Structure Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMeshComponent*> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<UStaticMeshComponent*> ExteriorDetails;

    // Materials and Appearance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* IntactMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* DamagedMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* RuinsMaterial;

    // Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TArray<class UPointLightComponent*> InteriorLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    class UPointLightComponent* FirePitLight;

    // Interaction System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanEnter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanRest = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bCanStore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float RestEffectiveness = 1.0f;

    // Structure Management Functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(EArch_StructureType InStructureType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetStructureState(EArch_StructureState NewState);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanAccommodateOccupants(int32 RequestedOccupants) const;

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Interior")
    void SetupInteriorProps();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void UpdateInteriorLighting();

    UFUNCTION(BlueprintCallable, Category = "Interior")
    void ToggleFirePit(bool bEnable);

    // Interaction Functions
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool AttemptEntry(class APawn* InteractingPawn);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool AttemptRest(class APawn* InteractingPawn);

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool AttemptStorage(class APawn* InteractingPawn);

    // Visual Updates
    UFUNCTION(BlueprintCallable, Category = "Visuals")
    void UpdateMaterialBasedOnState();

    UFUNCTION(BlueprintCallable, Category = "Visuals")
    void AddWeatheringEffects();

    UFUNCTION(BlueprintCallable, Category = "Visuals")
    void AddVegetationOvergrowth();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetInteriorCenter() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDefenseValue() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsStructureUsable() const;

protected:
    // Internal state tracking
    float WeatheringAccumulation;
    float LastMaintenanceTime;
    int32 CurrentOccupants;
    bool bIsInitialized;

    // Helper functions
    void LoadStructureMesh();
    void SetupCollisionBounds();
    void CreateInteriorProps();
    void ConfigureLighting();
    void ApplyWeatheringEffects();
};