#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Arch_InteriorSystem.generated.h"

// Interior object types for Cretaceous period dwellings
UENUM(BlueprintType)
enum class EArch_InteriorObjectType : uint8
{
    SleepingArea    UMETA(DisplayName = "Sleeping Area"),
    FirePit         UMETA(DisplayName = "Fire Pit"),
    StorageArea     UMETA(DisplayName = "Storage Area"),
    ToolRack        UMETA(DisplayName = "Tool Rack"),
    TrophyWall      UMETA(DisplayName = "Trophy Wall"),
    WorkBench       UMETA(DisplayName = "Work Bench"),
    WaterStorage    UMETA(DisplayName = "Water Storage"),
    FoodCache       UMETA(DisplayName = "Food Cache")
};

// Interior decoration materials
UENUM(BlueprintType)
enum class EArch_DecorationMaterial : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    Bone            UMETA(DisplayName = "Bone"),
    Antler          UMETA(DisplayName = "Antler"),
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Clay            UMETA(DisplayName = "Clay"),
    Shell           UMETA(DisplayName = "Shell"),
    Feather         UMETA(DisplayName = "Feather")
};

// Interior object data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorObject
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    EArch_InteriorObjectType ObjectType = EArch_InteriorObjectType::SleepingArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    EArch_DecorationMaterial PrimaryMaterial = EArch_DecorationMaterial::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    float WearLevel = 0.0f; // 0.0 = new, 1.0 = heavily worn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Object")
    FString CustomDescription;

    FArch_InteriorObject()
    {
        ObjectType = EArch_InteriorObjectType::SleepingArea;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        PrimaryMaterial = EArch_DecorationMaterial::AnimalHide;
        WearLevel = 0.0f;
        bIsActive = true;
        CustomDescription = TEXT("");
    }
};

// Interior layout configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    TArray<FArch_InteriorObject> InteriorObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    float FloorArea = 100.0f; // Square meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    float CeilingHeight = 3.0f; // Meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    EArch_DecorationMaterial WallMaterial = EArch_DecorationMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    EArch_DecorationMaterial FloorMaterial = EArch_DecorationMaterial::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    bool bHasNaturalLighting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Layout")
    bool bHasFireLighting = true;

    FArch_InteriorLayout()
    {
        FloorArea = 100.0f;
        CeilingHeight = 3.0f;
        WallMaterial = EArch_DecorationMaterial::Stone;
        FloorMaterial = EArch_DecorationMaterial::Stone;
        bHasNaturalLighting = false;
        bHasFireLighting = true;
    }
};

/**
 * Interior System Component - Manages the interior layout and decoration of Cretaceous period dwellings
 * Handles object placement, wear simulation, and atmospheric effects for realistic prehistoric interiors
 */
UCLASS(ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_InteriorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_InteriorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Interior layout configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior System")
    FArch_InteriorLayout CurrentLayout;

    // Interior object spawning
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void SpawnInteriorObjects();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void ClearInteriorObjects();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void AddInteriorObject(EArch_InteriorObjectType ObjectType, FVector Location, EArch_DecorationMaterial Material);

    // Interior atmosphere
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void SetupInteriorLighting();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void UpdateFireLighting(bool bFireActive);

    // Wear and aging simulation
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void SimulateWearAndTear(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void SetObjectWearLevel(int32 ObjectIndex, float WearLevel);

    // Interior validation
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    bool ValidateInteriorLayout() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    float CalculateInteriorComfort() const;

    // Preset layouts
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void LoadHunterShelterLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void LoadFamilyCaveLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void LoadShamanHutLayout();

    // Interior object management
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    TArray<AActor*> GetSpawnedInteriorActors() const;

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void RefreshInteriorObjects();

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void ApplyInteriorWeatherEffects(float Humidity, float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Interior System")
    void UpdateInteriorAmbience();

private:
    // Spawned interior object actors
    UPROPERTY()
    TArray<AActor*> SpawnedObjects;

    // Interior lighting components
    UPROPERTY()
    class UPointLightComponent* FireLight;

    UPROPERTY()
    class UStaticMeshComponent* FirePitMesh;

    // Interior atmosphere
    float InteriorHumidity = 0.5f;
    float InteriorTemperature = 20.0f;
    float FireIntensity = 1.0f;

    // Helper functions
    void SpawnObjectAtLocation(const FArch_InteriorObject& ObjectData);
    UStaticMesh* GetMeshForObjectType(EArch_InteriorObjectType ObjectType, EArch_DecorationMaterial Material);
    void SetupFirePit();
    void UpdateObjectMaterials();
    float CalculateObjectDensity() const;
};