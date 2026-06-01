#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_PlayerCharacterConfig.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PlayerAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    USkeletalMesh* BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    USkeletalMesh* HairMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    USkeletalMesh* ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    UMaterialInterface* ClothingMaterial;

    FChar_PlayerAppearance()
    {
        BodyMesh = nullptr;
        SkinMaterial = nullptr;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairMesh = nullptr;
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        ClothingMesh = nullptr;
        ClothingMaterial = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerCharacterConfig : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerCharacterConfig();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_PlayerAppearance PlayerAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
    float JumpHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraBoomLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    FRotator CameraBoomAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraFOV;

    UFUNCTION(BlueprintCallable, Category = "Character Config")
    void ApplyAppearanceToCharacter(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Config")
    void SetDefaultTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Config")
    void SetDefaultMovementSettings();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

#include "Char_PlayerCharacterConfig.generated.h"