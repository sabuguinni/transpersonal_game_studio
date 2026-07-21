#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "Anim_TribalShowcaseManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_TribalCharacterType : uint8
{
    Warrior     UMETA(DisplayName = "Tribal Warrior"),
    Healer      UMETA(DisplayName = "Tribal Healer"),
    Hunter      UMETA(DisplayName = "Tribal Hunter"),
    Chief       UMETA(DisplayName = "Tribal Chief"),
    Child       UMETA(DisplayName = "Tribal Child")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_TribalShowcaseData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    EAnim_TribalCharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    TArray<class UAnimMontage*> IdleAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    TArray<class UAnimMontage*> ActionAnimations;

    FAnim_TribalShowcaseData()
    {
        CharacterType = EAnim_TribalCharacterType::Warrior;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        CharacterName = TEXT("Tribal Character");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_TribalShowcaseManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_TribalShowcaseManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tribal character showcase management
    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    void CreateTribalShowcase();

    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    void SpawnTribalCharacter(const FAnim_TribalShowcaseData& ShowcaseData);

    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    void SetupTribalAnimations(ATranspersonalCharacter* Character, EAnim_TribalCharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    void PlayTribalIdleAnimation(ATranspersonalCharacter* Character, EAnim_TribalCharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    void PlayTribalActionAnimation(ATranspersonalCharacter* Character, EAnim_TribalCharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    void CycleTribalAnimations();

    UFUNCTION(BlueprintCallable, Category = "Tribal Showcase")
    TArray<ATranspersonalCharacter*> GetSpawnedTribalCharacters() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    TArray<FAnim_TribalShowcaseData> TribalShowcaseConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    TArray<ATranspersonalCharacter*> SpawnedTribalCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    float AnimationCycleInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    bool bAutoPlayAnimations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Showcase")
    bool bRandomizeAnimationTiming;

private:
    float LastAnimationCycleTime;
    int32 CurrentAnimationIndex;

    void InitializeTribalShowcaseConfigs();
    FAnim_TribalShowcaseData CreateWarriorConfig();
    FAnim_TribalShowcaseData CreateHealerConfig();
    FAnim_TribalShowcaseData CreateHunterConfig();
    FAnim_TribalShowcaseData CreateChiefConfig();
    FAnim_TribalShowcaseData CreateChildConfig();
};