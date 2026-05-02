#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "../SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave UMETA(DisplayName = "Cave Dwelling"),
    TreePlatform UMETA(DisplayName = "Tree Platform"),
    StoneCircle UMETA(DisplayName = "Stone Circle"),
    CliffDwelling UMETA(DisplayName = "Cliff Dwelling"),
    Underground UMETA(DisplayName = "Underground Bunker")
};

USTRUCT(BlueprintType)
struct FArch_ShelterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float SafetyRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 1;

    FArch_ShelterData()
    {
        ShelterType = EArch_ShelterType::Cave;
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        SafetyRadius = 500.0f;
        bHasFirePit = false;
        bHasStorage = false;
        MaxOccupants = 1;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ShelterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* InteriorLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    FArch_ShelterData ShelterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Configuration")
    TArray<FArch_ShelterData> ManagedShelters;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void CreateShelter(EArch_ShelterType ShelterType, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void DestroyShelter(int32 ShelterIndex);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    bool IsLocationSafe(FVector TestLocation, float TestRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    FArch_ShelterData GetNearestShelter(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void AddFirePit(int32 ShelterIndex);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void AddStorage(int32 ShelterIndex);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void SetupInteriorLighting(EArch_ShelterType ShelterType);

    UFUNCTION(BlueprintCallable, Category = "Shelter Management")
    void ValidateAllShelters();

    UFUNCTION(BlueprintCallable, Category = "Shelter Management", CallInEditor)
    void CreateDefaultShelters();

private:
    void InitializeShelterMesh(EArch_ShelterType ShelterType);
    void SetupShelterLighting(EArch_ShelterType ShelterType);
    bool CheckShelterSafety(const FArch_ShelterData& Shelter);
    void UpdateShelterVisuals();
};