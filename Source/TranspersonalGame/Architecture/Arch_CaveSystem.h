#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_CaveSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_CaveType : uint8
{
    ShallowCave     UMETA(DisplayName = "Shallow Cave"),
    DeepCavern      UMETA(DisplayName = "Deep Cavern"),
    TunnelSystem    UMETA(DisplayName = "Tunnel System"),
    UndergroundLake UMETA(DisplayName = "Underground Lake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_CaveProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    EArch_CaveType CaveType = EArch_CaveType::ShallowCave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float Temperature = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float Humidity = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasCaveArt = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    int32 MaxOccupants = 4;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CaveSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_CaveSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveEntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveInteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    FArch_CaveProperties CaveProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<FVector> ShelterSpots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<FVector> StorageAreas;

public:
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void InitializeCave(EArch_CaveType InCaveType);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool CanProvideShel ter() const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    FVector GetNearestShelterSpot(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void AddCaveArt(FVector Location, int32 ArtType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Cave System")
    void OnPlayerEnterCave();

    UFUNCTION(BlueprintImplementableEvent, Category = "Cave System")
    void OnPlayerExitCave();

private:
    void SetupCaveGeometry();
    void ConfigureAmbientSounds();
    void GenerateShelterSpots();
};