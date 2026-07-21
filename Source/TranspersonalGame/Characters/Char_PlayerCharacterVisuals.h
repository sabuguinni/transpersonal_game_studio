#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Char_PlayerCharacterVisuals.generated.h"

UENUM(BlueprintType)
enum class EChar_CharacterGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Tanned      UMETA(DisplayName = "Tanned")
};

UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    Short       UMETA(DisplayName = "Short"),
    Long        UMETA(DisplayName = "Long"),
    Braided     UMETA(DisplayName = "Braided"),
    Shaved      UMETA(DisplayName = "Shaved")
};

USTRUCT(BlueprintType)
struct FChar_CharacterAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_CharacterGender Gender = EChar_CharacterGender::Male;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle = EChar_HairStyle::Long;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.0f;

    FChar_CharacterAppearance()
    {
        Gender = EChar_CharacterGender::Male;
        SkinTone = EChar_SkinTone::Medium;
        HairStyle = EChar_HairStyle::Long;
        HairColor = FLinearColor::Black;
        BodyMass = 1.0f;
        Height = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerCharacterVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerCharacterVisuals();

protected:
    virtual void BeginPlay() override;

    // Character appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_CharacterAppearance CurrentAppearance;

    // Mesh and material references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Assets")
    TSoftObjectPtr<USkeletalMesh> MaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Assets")
    TSoftObjectPtr<USkeletalMesh> FemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> HairMaterials;

    // Camera configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraBoomLength = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    FRotator CameraBoomRotation = FRotator(-20.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    float CameraFOV = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Settings")
    bool bUsePawnControlRotation = true;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetCharacterAppearance(const FChar_CharacterAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyGenderMesh(EChar_CharacterGender Gender);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplySkinTone(EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyHairStyle(EChar_HairStyle HairStyle, FLinearColor HairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBodyScale(float BodyMass, float Height);

    // Camera setup functions
    UFUNCTION(BlueprintCallable, Category = "Camera Setup")
    void ConfigureThirdPersonCamera();

    UFUNCTION(BlueprintCallable, Category = "Camera Setup")
    void SetCameraBoomSettings(float BoomLength, FRotator BoomRotation);

    UFUNCTION(BlueprintCallable, Category = "Camera Setup")
    void SetCameraFOV(float NewFOV);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void RefreshCharacterVisuals();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    USkeletalMeshComponent* GetCharacterMesh() const;

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    UCameraComponent* GetCameraComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    USpringArmComponent* GetCameraBoom() const;

private:
    // Internal helper functions
    void LoadDefaultAssets();
    void SetupCameraComponents();
    UMaterialInterface* GetSkinMaterialForTone(EChar_SkinTone SkinTone) const;
    UMaterialInterface* GetHairMaterialForStyle(EChar_HairStyle HairStyle) const;
};