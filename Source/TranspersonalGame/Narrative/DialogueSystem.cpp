
#include "DialogueSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNarr_DialogueSystem::UNarr_DialogueSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsPlayingDialogue = false;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
    ProximityTriggerRadius = 400.0f;
}

void UNarr_DialogueSystem::BeginPlay()
{
    Super::BeginPlay();
}

void UNarr_DialogueSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsPlayingDialogue || !ActiveSequence)
    {
        return;
    }

    LineTimer -= DeltaTime;
    if (LineTimer <= 0.0f)
    {
        AdvanceLine();
    }
}

void UNarr_DialogueSystem::TriggerDialogueSequence(FName SequenceID)
{
    for (FNarr_DialogueSequence& Seq : DialogueSequences)
    {
        if (Seq.SequenceID == SequenceID)
        {
            if (Seq.bPlayOnce && Seq.bHasPlayed)
            {
                return;
            }

            ActiveSequence = &Seq;
            CurrentLineIndex = 0;
            bIsPlayingDialogue = true;
            Seq.bHasPlayed = true;

            if (Seq.Lines.Num() > 0)
            {
                LineTimer = Seq.Lines[0].DisplayDuration;

                // Play voice asset if available
                USoundBase* Voice = Seq.Lines[0].VoiceAsset.LoadSynchronous();
                if (Voice && GetWorld())
                {
                    UGameplayStatics::PlaySound2D(GetWorld(), Voice);
                }
            }
            else
            {
                StopDialogue();
            }
            return;
        }
    }
}

void UNarr_DialogueSystem::AdvanceLine()
{
    if (!ActiveSequence)
    {
        StopDialogue();
        return;
    }

    CurrentLineIndex++;

    if (CurrentLineIndex >= ActiveSequence->Lines.Num())
    {
        StopDialogue();
        return;
    }

    const FNarr_DialogueLine& Line = ActiveSequence->Lines[CurrentLineIndex];
    LineTimer = Line.DisplayDuration;

    USoundBase* Voice = Line.VoiceAsset.LoadSynchronous();
    if (Voice && GetWorld())
    {
        UGameplayStatics::PlaySound2D(GetWorld(), Voice);
    }
}

void UNarr_DialogueSystem::StopDialogue()
{
    bIsPlayingDialogue = false;
    ActiveSequence = nullptr;
    CurrentLineIndex = 0;
    LineTimer = 0.0f;
}

FNarr_DialogueLine UNarr_DialogueSystem::GetCurrentLine() const
{
    if (ActiveSequence && CurrentLineIndex < ActiveSequence->Lines.Num())
    {
        return ActiveSequence->Lines[CurrentLineIndex];
    }
    return FNarr_DialogueLine();
}

void UNarr_DialogueSystem::CheckProximityTrigger(FVector PlayerLocation)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    float Distance = FVector::Dist(Owner->GetActorLocation(), PlayerLocation);
    if (Distance <= ProximityTriggerRadius)
    {
        for (FNarr_DialogueSequence& Seq : DialogueSequences)
        {
            if (!Seq.bHasPlayed || !Seq.bPlayOnce)
            {
                bool bHasProximityLine = false;
                for (const FNarr_DialogueLine& Line : Seq.Lines)
                {
                    if (Line.Trigger == ENarr_DialogueTrigger::OnProximity)
                    {
                        bHasProximityLine = true;
                        break;
                    }
                }
                if (bHasProximityLine)
                {
                    TriggerDialogueSequence(Seq.SequenceID);
                    return;
                }
            }
        }
    }
}
