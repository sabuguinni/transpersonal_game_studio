#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_DwellingFoundation.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_FoundationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float Width = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float Length = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float Height = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    int32 StoneBlockCount = 24;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float MossGrowth = 0.3f;

    FArch_FoundationData()
    {
        Width = 400.0f;
        Length = 600.0f;
        Height = 50.0f;
        StoneBlockCount = 24;
        WeatheringLevel = 0.5f;
        MossGrowth = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_DwellingFoundation : public AActor
{
    GENERATED_BODY()

public:
    AArch_DwellingFoundation();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FoundationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractionBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    FArch_FoundationData FoundationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* StoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyStoneMaterial;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void GenerateFoundation();

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void UpdateWeathering(float NewWeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void UpdateMossGrowth(float NewMossLevel);

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    void SetFoundationDimensions(float NewWidth, float NewLength, float NewHeight);

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    FVector GetFoundationCenter() const;

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    bool IsPointOnFoundation(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Foundation")
    TArray<FVector> GetCornerPositions() const;

private:
    void UpdateMaterialBasedOnWeathering();
    void SetupCollision();
    void CreateFoundationGeometry();
};