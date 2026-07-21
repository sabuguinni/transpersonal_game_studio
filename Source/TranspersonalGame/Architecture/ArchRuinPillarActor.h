#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArchRuinPillarActor.generated.h"

UENUM(BlueprintType)
enum class EArch_PillarState : uint8
{
    Intact       UMETA(DisplayName = "Intact"),
    Cracked      UMETA(DisplayName = "Cracked"),
    Collapsed    UMETA(DisplayName = "Collapsed"),
    Buried       UMETA(DisplayName = "Buried")
};

UENUM(BlueprintType)
enum class EArch_PillarMaterial : uint8
{
    Limestone    UMETA(DisplayName = "Limestone"),
    Basalt       UMETA(DisplayName = "Basalt"),
    Sandstone    UMETA(DisplayName = "Sandstone"),
    Granite      UMETA(DisplayName = "Granite")
};

USTRUCT(BlueprintType)
struct FArch_PillarConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_PillarState State = EArch_PillarState::Intact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArch_PillarMaterial Material = EArch_PillarMaterial::Limestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float HeightMeters = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float RadiusMeters = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float MossCoverage = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bHasVines = true;
};

/**
 * AArch_RuinPillarActor
 * Cretaceous era stone ruin pillar — a structural remnant of prehistoric
 * construction, worn by millennia of jungle growth and geological pressure.
 * Each pillar is a document of the civilization that built it.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_RuinPillarActor : public AActor
{
    GENERATED_BODY()

public:
    AArch_RuinPillarActor();

    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PillarMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* DebrisMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FArch_PillarConfig PillarConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|LOD")
    float MaxVisibleDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
    bool bCanBeUsedAsCover = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture|Gameplay")
    bool bCanBeClimbed = false;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetPillarState(EArch_PillarState NewState);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    EArch_PillarState GetPillarState() const;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyWeathering(float WeatheringIntensity);

    UFUNCTION(CallInEditor, Category = "Architecture")
    void RegeneratePillar();

private:
    void UpdateMeshForState();
    void UpdateMaterialParameters();
};
