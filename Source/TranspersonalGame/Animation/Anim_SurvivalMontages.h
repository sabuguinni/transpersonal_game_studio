#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Anim_SurvivalMontages.generated.h"

UENUM(BlueprintType)
enum class EAnim_SurvivalAction : uint8
{
    None            UMETA(DisplayName = "None"),
    GatherWood      UMETA(DisplayName = "Gather Wood"),
    CraftTool       UMETA(DisplayName = "Craft Tool"),
    HuntSpear       UMETA(DisplayName = "Hunt with Spear"),
    BuildShelter    UMETA(DisplayName = "Build Shelter"),
    EatFood         UMETA(DisplayName = "Eat Food"),
    DrinkWater      UMETA(DisplayName = "Drink Water"),
    MakeFire        UMETA(DisplayName = "Make Fire"),
    SkinAnimal      UMETA(DisplayName = "Skin Animal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MontageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    UAnimMontage* Montage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float PlayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Montage")
    float BlendOutTime;

    FAnim_MontageData()
    {
        Montage = nullptr;
        PlayRate = 1.0f;
        BlendInTime = 0.25f;
        BlendOutTime = 0.25f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_SurvivalMontages : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_SurvivalMontages();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Montages")
    TMap<EAnim_SurvivalAction, FAnim_MontageData> SurvivalMontages;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EAnim_SurvivalAction CurrentAction;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsPlayingMontage;

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool PlaySurvivalAction(EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    void StopCurrentAction();

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    bool IsActionAvailable(EAnim_SurvivalAction Action) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Actions")
    float GetActionDuration(EAnim_SurvivalAction Action) const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Actions")
    void OnSurvivalActionStarted(EAnim_SurvivalAction Action);

    UFUNCTION(BlueprintImplementableEvent, Category = "Survival Actions")
    void OnSurvivalActionCompleted(EAnim_SurvivalAction Action);

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void InitializeDefaultMontages();
};