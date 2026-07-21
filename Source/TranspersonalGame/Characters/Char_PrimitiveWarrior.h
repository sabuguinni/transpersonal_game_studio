#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_PrimitiveWarrior.generated.h"

/**
 * Primitive Cretaceous tribal warrior character
 * Represents early humans surviving in the dinosaur era
 * Features realistic prehistoric appearance with animal hide clothing
 * Stone tools and tribal markings for authentic period accuracy
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveWarrior : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveWarrior();

protected:
    virtual void BeginPlay() override;

    // Character appearance components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class UStaticMeshComponent* SpearMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class UStaticMeshComponent* BoneNecklaceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Appearance")
    class UStaticMeshComponent* StoneKnifeMesh;

    // Character customization properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* SpearMeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* BoneNecklaceAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UStaticMesh* StoneKnifeAsset;

    // Character stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TribalStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SurvivalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HuntingSkill;

public:
    virtual void Tick(float DeltaTime) override;

    // Character setup functions
    UFUNCTION(BlueprintCallable, Category = "Setup")
    void SetupTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Setup")
    void AttachTribalEquipment();

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void SetTribalStats(float Strength, float Experience, float Hunting);

    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetTribalStrength() const { return TribalStrength; }

    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetSurvivalExperience() const { return SurvivalExperience; }

    UFUNCTION(BlueprintPure, Category = "Stats")
    float GetHuntingSkill() const { return HuntingSkill; }
};