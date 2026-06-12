#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntityTemplate.h"
#include "MassCommonTypes.h"
#include "Engine/World.h"
#include "Crowd_MassEntityTraits.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bHasTarget = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 BehaviorState = 0; // 0=Idle, 1=Walking, 2=Running, 3=Fleeing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 GroupID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FlockingRadius = 200.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 LODLevel = 0; // 0=High, 1=Medium, 2=Low, 3=Culled

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LastUpdateTime = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_VisualFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    int32 MeshVariant = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float Scale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bCastShadow = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityTrait : public UMassEntityTraitBase
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityTrait();

    virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float MovementSpeed = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    float FlockingRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Settings")
    int32 MaxEntitiesPerGroup = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighLODDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumLODDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowLODDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    int32 MeshVariants = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Settings")
    bool bEnableShadowCasting = true;
};