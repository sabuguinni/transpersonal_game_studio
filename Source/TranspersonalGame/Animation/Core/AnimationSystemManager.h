#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "AnimationSystemManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAnimationSystem, Log, All);

/**
 * Sistema central de gestão de animação para o jogo Jurássico
 * Responsável por coordenar Motion Matching, IK, e variações procedurais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Motion Matching Database Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    TMap<FString, class UPoseSearchDatabase*> MotionMatchingDatabases;

    // IK System Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKTraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float FootIKInterpSpeed = 15.0f;

    // Procedural Animation Variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Variation")
    float MovementVariationScale = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Variation")
    int32 MaxAnimationVariants = 5;

public:
    // Motion Matching Functions
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    class UPoseSearchDatabase* GetMotionMatchingDatabase(const FString& DatabaseName);

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void RegisterMotionMatchingDatabase(const FString& DatabaseName, class UPoseSearchDatabase* Database);

    // IK System Functions
    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector CalculateFootIKOffset(AActor* Character, const FName& FootBoneName, const FVector& FootLocation);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    bool PerformFootIKTrace(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    // Procedural Variation Functions
    UFUNCTION(BlueprintCallable, Category = "Procedural Variation")
    float GenerateMovementVariation(int32 CharacterID, const FString& MovementType);

    UFUNCTION(BlueprintCallable, Category = "Procedural Variation")
    void ApplyCharacterSpecificAnimationTweaks(class UAnimInstance* AnimInstance, int32 CharacterID);

private:
    // Internal tracking
    TMap<int32, FRandomStream> CharacterRandomStreams;
    
    void InitializeCharacterVariation(int32 CharacterID);
    float GetCachedVariation(int32 CharacterID, const FString& VariationType);
    
    // Cache para optimização
    TMap<FString, float> VariationCache;
};