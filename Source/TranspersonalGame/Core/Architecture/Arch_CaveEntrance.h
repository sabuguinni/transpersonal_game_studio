#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "Arch_CaveEntrance.generated.h"

UENUM(BlueprintType)
enum class EArch_CaveType : uint8
{
    Natural_Limestone,
    Carved_Sandstone,
    Volcanic_Basalt,
    Weathered_Granite,
    Ancient_Ruins
};

USTRUCT(BlueprintType)
struct FArch_CaveProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    EArch_CaveType CaveType = EArch_CaveType::Natural_Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float EntranceWidth = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float EntranceHeight = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float InteriorDepth = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float TemperatureModifier = -5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasAncientCarvings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bHasNaturalLight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    float HumidityLevel = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Properties")
    bool bIsShelterSafe = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CaveEntrance : public AActor
{
    GENERATED_BODY()

public:
    AArch_CaveEntrance();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CaveEntranceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* InteriorWallsMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteriorVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* EntranceVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* InteriorLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    FArch_CaveProperties CaveProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    TArray<UStaticMesh*> CaveEntranceMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    TArray<UMaterialInterface*> CaveMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    bool bRandomizeOnSpawn = true;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetCaveType(EArch_CaveType NewType);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void RandomizeCaveAppearance();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetCaveDimensions(float Width, float Height, float Depth);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool IsPlayerInCave() const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    float GetInteriorTemperature() const;

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetAncientCarvings(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void UpdateInteriorLighting();

    UFUNCTION(BlueprintImplementableEvent, Category = "Cave Events")
    void OnPlayerEnterCave();

    UFUNCTION(BlueprintImplementableEvent, Category = "Cave Events")
    void OnPlayerExitCave();

protected:
    UFUNCTION()
    void OnEntranceVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEntranceVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    void SetupCaveMesh();
    void SetupInteriorVolume();
    void ApplyCaveTypeProperties();
    void UpdateCaveMaterials();

private:
    bool bPlayerInside = false;
    float BaseTemperature = 20.0f;
    
    UPROPERTY()
    TArray<AActor*> ActorsInside;
};