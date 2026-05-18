#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "ArchitecturalManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    Dwelling        UMETA(DisplayName = "Dwelling"),
    Storage         UMETA(DisplayName = "Storage"),
    Ritual          UMETA(DisplayName = "Ritual Site"),
    Defensive       UMETA(DisplayName = "Defensive Structure"),
    Workshop        UMETA(DisplayName = "Workshop")
};

UENUM(BlueprintType)
enum class EArch_MaterialType : uint8
{
    Stone           UMETA(DisplayName = "Stone"),
    Wood            UMETA(DisplayName = "Wood"),
    Thatch          UMETA(DisplayName = "Thatch"),
    Hide            UMETA(DisplayName = "Animal Hide"),
    Bone            UMETA(DisplayName = "Bone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::Dwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_MaterialType PrimaryMaterial = EArch_MaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Dimensions = FVector(400.0f, 400.0f, 300.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 Durability = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatherResistance = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasInterior = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    TArray<FString> InteriorElements;

    FArch_StructureData()
    {
        InteriorElements.Add("Floor");
        InteriorElements.Add("Walls");
        InteriorElements.Add("Ceiling");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> RoomCenters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> DoorwayPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FVector> WindowPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float RoomHeight = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasFireplace = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bHasStorageArea = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ArchitecturalStructure : public AActor
{
    GENERATED_BODY()

public:
    AArch_ArchitecturalStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FoundationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WallsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* RoofMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ThatchMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void InitializeStructure(const FArch_StructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetupInterior(const FArch_InteriorLayout& InLayout);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanEnterStructure() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<FVector> GetInteriorSpawnPoints() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetMaterialByType(EArch_MaterialType MaterialType);

    UFUNCTION(BlueprintPure, Category = "Architecture")
    float GetStructuralIntegrity() const;

    UFUNCTION(BlueprintPure, Category = "Architecture")
    bool IsStructureHabitable() const;

private:
    void UpdateMeshMaterials();
    void GenerateInteriorElements();
    void SetupCollision();
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UArch_ArchitecturalManager : public UObject
{
    GENERATED_BODY()

public:
    UArch_ArchitecturalManager();

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    static UArch_ArchitecturalManager* GetInstance();

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    AArch_ArchitecturalStructure* SpawnStructure(UWorld* World, const FVector& Location, 
        const FRotator& Rotation, const FArch_StructureData& StructureData);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    void RegisterStructure(AArch_ArchitecturalStructure* Structure);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    void UnregisterStructure(AArch_ArchitecturalStructure* Structure);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    TArray<AArch_ArchitecturalStructure*> GetStructuresInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    void ApplyWeatherToAllStructures(float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    int32 GetTotalStructureCount() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture Manager")
    void CleanupDestroyedStructures();

protected:
    UPROPERTY()
    TArray<AArch_ArchitecturalStructure*> RegisteredStructures;

private:
    static UArch_ArchitecturalManager* Instance;
};