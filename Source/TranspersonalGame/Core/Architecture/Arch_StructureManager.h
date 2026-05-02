#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_StructureManager.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    CaveDwelling,
    TreePlatform,
    StoneCircle,
    CliffRuin,
    SimpleHut,
    WoodBridge,
    StoneWall,
    Watchtower
};

UENUM(BlueprintType)
enum class EArch_StructureState : uint8
{
    Pristine,
    Weathered,
    Damaged,
    Ruined,
    Overgrown
};

USTRUCT(BlueprintType)
struct FArch_StructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArch_StructureType StructureType = EArch_StructureType::CaveDwelling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EArch_StructureState CurrentState = EArch_StructureState::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StructureScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StructuralIntegrity = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeEntered = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bProvidesWeatherShelter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxOccupants = 1;

    FArch_StructureData()
    {
        StructureType = EArch_StructureType::CaveDwelling;
        CurrentState = EArch_StructureState::Weathered;
        StructureScale = FVector(1.0f, 1.0f, 1.0f);
        StructuralIntegrity = 75.0f;
        bCanBeEntered = true;
        bProvidesWeatherShelter = false;
        MaxOccupants = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_StructureManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructureManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    FArch_StructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    TArray<UStaticMesh*> StructureMeshVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    TArray<UMaterialInterface*> WeatheringMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    bool bAutoSelectMeshByType = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Settings")
    float WeatheringRate = 0.1f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void SetStructureState(EArch_StructureState NewState);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool CanPlayerEnter() const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    bool ProvidesWeatherProtection() const;

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void OnPlayerEnter();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    void OnPlayerExit();

    UFUNCTION(BlueprintCallable, Category = "Structure Management")
    FVector GetSafeSpawnLocation() const;

    UFUNCTION(BlueprintPure, Category = "Structure Info")
    EArch_StructureType GetStructureType() const { return StructureData.StructureType; }

    UFUNCTION(BlueprintPure, Category = "Structure Info")
    EArch_StructureState GetStructureState() const { return StructureData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Structure Info")
    float GetStructuralIntegrity() const { return StructureData.StructuralIntegrity; }

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateMeshBasedOnType();
    void UpdateMaterialBasedOnState();
    void SetupInteractionVolume();

private:
    UPROPERTY()
    TArray<AActor*> CurrentOccupants;

    float WeatheringTimer = 0.0f;
    bool bPlayerInside = false;
};