#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Arch_DwellingManager.generated.h"

UENUM(BlueprintType)
enum class EArch_DwellingType : uint8
{
    SimpleHut       UMETA(DisplayName = "Simple Hut"),
    StoneArchway    UMETA(DisplayName = "Stone Archway"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    TribalLodge     UMETA(DisplayName = "Tribal Lodge")
};

UENUM(BlueprintType)
enum class EArch_DwellingState : uint8
{
    Pristine        UMETA(DisplayName = "Pristine"),
    Weathered       UMETA(DisplayName = "Weathered"),
    Damaged         UMETA(DisplayName = "Damaged"),
    Ruined          UMETA(DisplayName = "Ruined"),
    Collapsed       UMETA(DisplayName = "Collapsed")
};

USTRUCT(BlueprintType)
struct FArch_DwellingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingType DwellingType = EArch_DwellingType::SimpleHut;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    EArch_DwellingState CurrentState = EArch_DwellingState::Pristine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    float WeatheringRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling")
    int32 MaxOccupants = 4;

    FArch_DwellingConfig()
    {
        DwellingType = EArch_DwellingType::SimpleHut;
        CurrentState = EArch_DwellingState::Pristine;
        StructuralIntegrity = 100.0f;
        WeatheringRate = 0.1f;
        bHasFirePit = true;
        MaxOccupants = 4;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_DwellingManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_DwellingManager();

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
    UStaticMeshComponent* FirePitMesh;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dwelling Config")
    FArch_DwellingConfig DwellingConfig;

    // Materials for different states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> StateMaterials;

    // Dwelling Management
    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void InitializeDwelling(EArch_DwellingType Type, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void UpdateStructuralIntegrity(float DeltaIntegrity);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void ApplyWeathering(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void SetDwellingState(EArch_DwellingState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    bool CanAccommodateOccupants(int32 OccupantCount) const;

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void CreateInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void SpawnFirePit();

    UFUNCTION(BlueprintCallable, Category = "Dwelling")
    void AddSleepingAreas(int32 Count);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Dwelling")
    EArch_DwellingType GetDwellingType() const { return DwellingConfig.DwellingType; }

    UFUNCTION(BlueprintPure, Category = "Dwelling")
    EArch_DwellingState GetCurrentState() const { return DwellingConfig.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Dwelling")
    float GetStructuralIntegrity() const { return DwellingConfig.StructuralIntegrity; }

private:
    void UpdateMaterialBasedOnState();
    void HandleStructuralCollapse();
    FVector GetRandomInteriorPosition() const;
};