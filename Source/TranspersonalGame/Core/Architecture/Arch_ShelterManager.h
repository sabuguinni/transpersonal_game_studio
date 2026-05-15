#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Arch_ShelterManager.generated.h"

UENUM(BlueprintType)
enum class EArch_ShelterType : uint8
{
    Cave            UMETA(DisplayName = "Cave Shelter"),
    Platform        UMETA(DisplayName = "Elevated Platform"),
    Underground     UMETA(DisplayName = "Underground Bunker"),
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    TreeHouse       UMETA(DisplayName = "Tree House"),
    RockOverhang    UMETA(DisplayName = "Rock Overhang")
};

USTRUCT(BlueprintType)
struct FArch_ShelterConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    EArch_ShelterType ShelterType = EArch_ShelterType::Cave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    FVector Dimensions = FVector(400, 400, 300);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float ProtectionLevel = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasFirePit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    bool bHasStorage = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter")
    float CamouflageLevel = 0.6f;
};

USTRUCT(BlueprintType)
struct FArch_InteriorElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FString ElementName = TEXT("Fire Pit");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsEssential = true;
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
    UStaticMeshComponent* MainStructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shelter Config")
    FArch_ShelterConfig ShelterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    TArray<FArch_InteriorElement> InteriorElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WoodMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ThatchMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void ConstructShelter();

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void AddInteriorElement(const FArch_InteriorElement& Element);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void RemoveInteriorElement(const FString& ElementName);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    bool CanAccommodateOccupants(int32 OccupantCount) const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionFromWeather() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    float GetProtectionFromPredators() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    FVector GetOptimalFirePitLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    TArray<FVector> GetStorageLocations() const;

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    void SetShelterType(EArch_ShelterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Shelter")
    EArch_ShelterType GetShelterType() const { return ShelterConfig.ShelterType; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void RegenerateShelter();

private:
    void CreateCaveShelter();
    void CreatePlatformShelter();
    void CreateUndergroundBunker();
    void CreateStoneCircle();
    void CreateTreeHouse();
    void CreateRockOverhang();

    void SpawnInteriorElements();
    void ApplyMaterialsBasedOnType();
    void SetupCollision();

    UPROPERTY()
    TArray<UStaticMeshComponent*> StructuralComponents;

    UPROPERTY()
    TArray<UStaticMeshComponent*> InteriorComponents;
};