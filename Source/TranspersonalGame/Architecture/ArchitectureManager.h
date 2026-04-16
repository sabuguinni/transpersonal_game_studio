#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "ArchitectureManager.generated.h"

class AActor;
class UStaticMesh;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    EArch_ShelterType ShelterType;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    FVector Location;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    FRotator Rotation;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    float StructuralIntegrity;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    int32 MaxOccupants;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    bool bIsWeatherProof;

    UPROPERTY(BlueprintReadOnly, Category = "Shelter")
    TArray<AActor*> StructuralComponents;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::SimpleHut;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        StructuralIntegrity = 100.0f;
        MaxOccupants = 2;
        bIsWeatherProof = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_InteriorSpace
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    EArch_InteriorType InteriorType;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    FVector Bounds;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    float Temperature;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    float Humidity;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    bool bHasFireplace;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    TArray<AActor*> Furniture;

    UPROPERTY(BlueprintReadOnly, Category = "Interior")
    TArray<AActor*> StorageContainers;

    FArch_InteriorSpace()
    {
        InteriorType = EArch_InteriorType::LivingArea;
        Bounds = FVector(400.0f, 400.0f, 300.0f);
        Temperature = 20.0f;
        Humidity = 50.0f;
        bHasFireplace = false;
    }
};

/**
 * Architecture Manager - Handles all building construction and interior design
 * Manages shelter placement, structural integrity, and interior furnishing
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UArchitectureManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UArchitectureManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Shelter Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* CreateShelter(EArch_ShelterType ShelterType, const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool DestroyShelter(AActor* ShelterActor);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<AActor*> GetAllShelters() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FArch_ShelterData GetShelterData(AActor* ShelterActor) const;

    // Interior Management
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool SetupInterior(AActor* ShelterActor, EArch_InteriorType InteriorType);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    AActor* PlaceFurniture(AActor* ShelterActor, EArch_FurnitureType FurnitureType, const FVector& RelativeLocation);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RemoveFurniture(AActor* FurnitureActor);

    // Construction System
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool CanBuildAtLocation(const FVector& Location, EArch_ShelterType ShelterType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    TArray<EArch_MaterialType> GetRequiredMaterials(EArch_ShelterType ShelterType) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float CalculateConstructionTime(EArch_ShelterType ShelterType) const;

    // Structural Integrity
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void UpdateStructuralIntegrity(AActor* ShelterActor, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool RepairStructure(AActor* ShelterActor, float RepairAmount);

    // Weather Protection
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationProtectedFromWeather(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    float GetShelterEffectiveness(AActor* ShelterActor, EArch_WeatherType WeatherType) const;

    // Editor Tools
    UFUNCTION(CallInEditor, Category = "Architecture")
    void GenerateTestShelters();

    UFUNCTION(CallInEditor, Category = "Architecture")
    void ClearAllShelters();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<AActor*, FArch_ShelterData> RegisteredShelters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
    TMap<AActor*, FArch_InteriorSpace> InteriorSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EArch_ShelterType, UStaticMesh*> ShelterMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TMap<EArch_FurnitureType, UStaticMesh*> FurnitureMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float StructuralDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WeatherDamageMultiplier;

private:
    AActor* SpawnShelterComponents(EArch_ShelterType ShelterType, const FVector& Location, const FRotator& Rotation);
    void SetupShelterCollision(AActor* ShelterActor);
    void ApplyWeatherDamage(AActor* ShelterActor, float DeltaTime);
    bool ValidateBuildLocation(const FVector& Location) const;
};