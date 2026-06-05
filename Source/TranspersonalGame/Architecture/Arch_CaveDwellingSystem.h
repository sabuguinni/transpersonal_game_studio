#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Arch_CaveDwellingSystem.generated.h"

UENUM(BlueprintType)
enum class EArch_CaveType : uint8
{
    ShallowShelter    UMETA(DisplayName = "Shallow Shelter"),
    DeepCave         UMETA(DisplayName = "Deep Cave"),
    TunnelSystem     UMETA(DisplayName = "Tunnel System"),
    UndergroundChamber UMETA(DisplayName = "Underground Chamber")
};

USTRUCT(BlueprintType)
struct FArch_CaveProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    EArch_CaveType CaveType = EArch_CaveType::ShallowShelter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float Temperature = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float Humidity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasWaterSource = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    int32 MaxOccupants = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float ShelterValue = 0.7f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CaveDwellingSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_CaveDwellingSystem();

protected:
    virtual void BeginPlay() override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveEntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveInteriorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    // Cave Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    FArch_CaveProperties CaveProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    TArray<AActor*> InteriorProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave System")
    int32 CurrentOccupants = 0;

public:
    virtual void Tick(float DeltaTime) override;

    // Cave System Functions
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void EnterCave(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void ExitCave(AActor* Character);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool CanEnterCave() const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetShelterValue() const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetupCaveInterior();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void UpdateAmbientEffects();

protected:
    // Event Functions
    UFUNCTION()
    void OnInteriorTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnInteriorTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    // Internal Functions
    void InitializeCaveComponents();
    void SetupTriggerEvents();
    void LoadCaveMeshes();
    void ConfigureAmbientAudio();
};