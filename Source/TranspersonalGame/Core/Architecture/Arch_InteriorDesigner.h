#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Arch_InteriorDesigner.generated.h"

UENUM(BlueprintType)
enum class EArch_InteriorElement : uint8
{
    FirePit          UMETA(DisplayName = "Fire Pit"),
    SleepingArea     UMETA(DisplayName = "Sleeping Area"),
    ToolStorage      UMETA(DisplayName = "Tool Storage"),
    FoodStorage      UMETA(DisplayName = "Food Storage"),
    WaterContainer   UMETA(DisplayName = "Water Container"),
    WorkBench        UMETA(DisplayName = "Work Bench"),
    ArtWall          UMETA(DisplayName = "Art Wall"),
    EntryWay         UMETA(DisplayName = "Entry Way")
};

USTRUCT(BlueprintType)
struct FArch_InteriorElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    EArch_InteriorElement ElementType = EArch_InteriorElement::FirePit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector RelativeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FRotator RelativeRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
    float FunctionalRadius = 200.0f;

    FArch_InteriorElement()
    {
        ElementType = EArch_InteriorElement::FirePit;
        RelativeLocation = FVector::ZeroVector;
        RelativeRotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        bIsActive = true;
        FunctionalRadius = 200.0f;
    }
};

USTRUCT(BlueprintType)
struct FArch_InteriorLayout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    TArray<FArch_InteriorElement> Elements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FVector InteriorBounds = FVector(1000.0f, 1000.0f, 400.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    float AmbientLightLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
    FLinearColor AmbientLightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    FArch_InteriorLayout()
    {
        Elements.Empty();
        InteriorBounds = FVector(1000.0f, 1000.0f, 400.0f);
        AmbientLightLevel = 0.3f;
        AmbientLightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_InteriorDesigner : public AActor
{
    GENERATED_BODY()

public:
    AArch_InteriorDesigner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* InteriorRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteriorBounds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* AmbientLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior Design")
    FArch_InteriorLayout InteriorLayout;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior Design")
    TArray<UStaticMeshComponent*> ElementMeshes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interior Design")
    TArray<UPointLightComponent*> ElementLights;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void GenerateInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void ClearInteriorLayout();

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void AddInteriorElement(EArch_InteriorElement ElementType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void RemoveInteriorElement(int32 ElementIndex);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    bool IsLocationInsideInterior(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    EArch_InteriorElement GetNearestElementType(const FVector& Location, float& Distance);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void SetInteriorBounds(const FVector& NewBounds);

    UFUNCTION(BlueprintCallable, Category = "Interior Design")
    void UpdateAmbientLighting();

protected:
    void CreateElementMesh(const FArch_InteriorElement& Element, int32 ElementIndex);
    void CreateElementLight(const FArch_InteriorElement& Element, int32 ElementIndex);
    UStaticMesh* GetMeshForElementType(EArch_InteriorElement ElementType);
    FLinearColor GetLightColorForElementType(EArch_InteriorElement ElementType);
    float GetLightIntensityForElementType(EArch_InteriorElement ElementType);
    void GenerateDefaultLayout();
};