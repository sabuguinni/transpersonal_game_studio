#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"),
    TreeHollow      UMETA(DisplayName = "Tree Hollow"),
    StoneHut        UMETA(DisplayName = "Stone Hut"),
    LeanTo          UMETA(DisplayName = "Lean-To"),
    Windbreak       UMETA(DisplayName = "Windbreak")
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::CaveEntrance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector ShelterSize = FVector(400.0f, 300.0f, 250.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float WeatherProtection = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float TemperatureBonus = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::CaveEntrance;
        ShelterSize = FVector(400.0f, 300.0f, 250.0f);
        ProtectionRadius = 500.0f;
        WeatherProtection = 0.8f;
        TemperatureBonus = 5.0f;
        bHasFirePit = false;
        bIsOccupied = false;
        MaxOccupants = 4;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString ElementName = TEXT("FirePit");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsActive = true;

    FArch_InteriorElement()
    {
        ElementName = TEXT("FirePit");
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_ShelterManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_ShelterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core shelter properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<FArch_InteriorElement> InteriorElements;

    // Mesh components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Meshes")
    class UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Meshes")
    TArray<class UStaticMeshComponent*> InteriorMeshes;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Materials")
    class UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Materials")
    class UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Materials")
    class UMaterialInterface* MossMaterial;

    // Shelter management functions
    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void InitializeShelter(EArch_ShelterType InShelterType, FVector InSize);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void AddInteriorElement(const FArch_InteriorElement& NewElement);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void RemoveInteriorElement(const FString& ElementName);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    bool CanProvideProtection(FVector TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    float GetProtectionLevel(FVector TestLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void SetOccupancy(bool bOccupied, int32 NumOccupants = 1);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void ToggleFirePit(bool bActive);

    // Biome-specific shelter generation
    UFUNCTION(BlueprintCallable, Category = "Shelter Generation")
    void GenerateBiomeSpecificShelter(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Shelter Generation")
    void ApplyWeatheringEffects(float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Shelter Generation")
    void UpdateShelterCondition(float DeltaTime);

private:
    // Internal state
    float CurrentCondition = 1.0f;
    float LastMaintenanceTime = 0.0f;
    TArray<AActor*> CurrentOccupants;

    // Helper functions
    void CreateMainStructure();
    void CreateInteriorElements();
    void ApplyMaterialsBasedOnType();
    void SetupCollisionAndPhysics();
    FVector GetBiomeSpecificSize(EBiomeType BiomeType) const;
    TArray<FArch_InteriorElement> GetBiomeSpecificInterior(EBiomeType BiomeType) const;
};