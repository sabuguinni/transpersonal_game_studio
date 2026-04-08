#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterArchetypeSystem.h"
#include "MetaHumanCharacterComponent.generated.h"

class USkeletalMeshComponent;
class UAnimBlueprint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterAppearanceChanged, const FCharacterVisualTraits&, NewTraits);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMetaHumanCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMetaHumanCharacterComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
        FActorComponentTickFunction* ThisTickFunction) override;

    // Character archetype and visual data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    TSoftObjectPtr<UCharacterArchetype> CharacterArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    FCharacterVisualTraits CurrentVisualTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Setup")
    FCharacterClothing CurrentClothing;

    // MetaHuman mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Setup")
    TSoftObjectPtr<USkeletalMesh> BaseMaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Setup")
    TSoftObjectPtr<USkeletalMesh> BaseFemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Setup")
    TSoftObjectPtr<UAnimBlueprint> CharacterAnimBlueprint;

    // Survival state tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Tracking")
    ESurvivalCondition CurrentSurvivalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Tracking")
    int32 DaysSurvived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Tracking")
    float HealthCondition = 1.0f; // 0 = dying, 1 = perfect health

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Tracking")
    float MentalState = 1.0f; // 0 = broken, 1 = strong

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Character Events")
    FOnCharacterAppearanceChanged OnAppearanceChanged;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void ApplyArchetype(UCharacterArchetype* NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void UpdateVisualTraits(const FCharacterVisualTraits& NewTraits);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void UpdateClothing(const FCharacterClothing& NewClothing);

    UFUNCTION(BlueprintCallable, Category = "Survival System")
    void AdvanceSurvivalState(int32 DaysElapsed);

    UFUNCTION(BlueprintCallable, Category = "Survival System")
    void ApplyInjury(float InjurySeverity);

    UFUNCTION(BlueprintCallable, Category = "Survival System")
    void ApplyPsychologicalStress(float StressLevel);

    // Visual update functions
    UFUNCTION(BlueprintCallable, Category = "Visual Updates")
    void RefreshCharacterMesh();

    UFUNCTION(BlueprintCallable, Category = "Visual Updates")
    void ApplySurvivalWear();

    UFUNCTION(BlueprintCallable, Category = "Visual Updates")
    void UpdateClothingCondition();

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Character Info")
    FString GetCharacterDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Character Info")
    FText GetCharacterDescription() const;

    UFUNCTION(BlueprintPure, Category = "Character Info")
    float GetOverallCondition() const;

private:
    // Internal mesh management
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> CachedMeshComponent;

    // Survival state calculations
    void CalculateSurvivalEffects();
    void UpdateMaterialParameters();
    
    // MetaHuman integration
    void SetupMetaHumanMesh();
    void ApplyBodyMorphs();
    void ApplyFacialMorphs();
    void ApplyClothingMeshes();
};