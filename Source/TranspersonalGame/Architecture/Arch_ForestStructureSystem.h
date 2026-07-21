#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_ForestStructureSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_ForestStructureType : uint8
{
    LogShelter      UMETA(DisplayName = "Log Shelter"),
    TreePlatform    UMETA(DisplayName = "Tree Platform"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    WoodBridge      UMETA(DisplayName = "Wood Bridge"),
    StorageCache    UMETA(DisplayName = "Storage Cache")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ForestStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_ForestStructureType StructureType = EArch_ForestStructureType::LogShelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bIsWeatherproof = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    int32 MaxOccupants = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float CamouflageLevel = 0.5f;

    FArch_ForestStructureData()
    {
        StructureType = EArch_ForestStructureType::LogShelter;
        StructuralIntegrity = 100.0f;
        bIsWeatherproof = false;
        MaxOccupants = 1;
        CamouflageLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_ForestStructureSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_ForestStructureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Structure")
    FArch_ForestStructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Structure")
    TArray<UStaticMesh*> LogMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Structure")
    TArray<UMaterialInterface*> WeatheredMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Structure")
    bool bAutoGenerateStructure = true;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Forest Structure")
    void SetStructureType(EArch_ForestStructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Forest Structure")
    void ApplyWeatherDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Forest Structure")
    bool CanProvideShelte() const;

    UFUNCTION(BlueprintCallable, Category = "Forest Structure")
    void GenerateStructureComponents();

    UFUNCTION(BlueprintCallable, Category = "Forest Structure")
    float GetCamouflageEffectiveness() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Forest Structure")
    void OnStructureEntered(AActor* EnteringActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Forest Structure")
    void OnStructureExited(AActor* ExitingActor);

protected:
    UFUNCTION()
    void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    void InitializeStructureComponents();
    void SetupInteractionVolume();
    void ApplyMossAndWeathering();
    
    float WeatheringTimer = 0.0f;
    TArray<AActor*> CurrentOccupants;
};