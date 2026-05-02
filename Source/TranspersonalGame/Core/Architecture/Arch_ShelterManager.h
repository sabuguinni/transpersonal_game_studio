#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveDwelling        UMETA(DisplayName = "Cave Dwelling"),
    TreePlatform        UMETA(DisplayName = "Tree Platform"),
    StoneCircle         UMETA(DisplayName = "Stone Circle"),
    CliffDwelling       UMETA(DisplayName = "Cliff Dwelling"),
    RockOverhang        UMETA(DisplayName = "Rock Overhang"),
    TemporaryCamp       UMETA(DisplayName = "Temporary Camp")
};

USTRUCT(BlueprintType)
struct FArch_ShelterProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionLevel = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureBonus = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bIsDefensive = false;

    FArch_ShelterProperties()
    {
        ShelterType = EArch_ShelterType::CaveDwelling;
        ProtectionLevel = 0.8f;
        TemperatureBonus = 15.0f;
        WeatherProtection = 0.9f;
        MaxOccupants = 2;
        bHasFirePit = true;
        bHasStorage = false;
        bIsDefensive = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ShelterManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ProtectionZone;

    // Shelter Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterProperties ShelterProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<UStaticMesh*> ShelterMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    UStaticMesh* FirePitMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<FVector> StorageLocations;

    // Gameplay Functions
    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanPlayerEnter(class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void OnPlayerEnterShelter(class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void OnPlayerExitShelter(class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetTemperatureBonus() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool IsPlayerProtected(class ATranspersonalCharacter* Player) const;

    // Construction System
    UFUNCTION(BlueprintCallable, Category = "Construction")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void UpdateShelterMesh();

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void PlaceFirePit(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Construction")
    void AddStorageLocation(FVector Location);

    // Validation and Placement
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ValidatePlacement();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void AutoConfigureForTerrain();

private:
    // Internal state
    TArray<class ATranspersonalCharacter*> CurrentOccupants;
    
    bool bIsInitialized = false;
    float LastValidationTime = 0.0f;

    // Helper functions
    void InitializeShelterMesh();
    void SetupCollisionVolumes();
    EArch_ShelterType DetermineBestShelterType(FVector Location);
    bool IsLocationSuitableForShelter(FVector Location, EArch_ShelterType ShelterType);
};